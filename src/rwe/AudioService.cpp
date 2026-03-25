#include "AudioService.h"

namespace rwe
{
    AudioService::AudioService(
        SdlContext* sdlContext,
        SdlMixerContext* sdlMixerContext,
        AbstractVirtualFileSystem* fileSystem)
        : sdlContext(sdlContext),
          sdlMixerContext(sdlMixerContext),
          fileSystem(fileSystem)
    {
    }

    void AudioService::allocateTracks(unsigned int count)
    {
        tracks.reserve(count);
        for (unsigned int i = tracks.size(); i < count; ++i)
        {
            auto track = sdlMixerContext->createTrack();
            if (!track)
            {
                throw std::runtime_error("Failed to create mixer track");
            }
            tracks.push_back(std::move(track));
            setupTrackCallback(i);
        }
    }

    void AudioService::setupTrackCallback(int trackIndex)
    {
        sdlMixerContext->setTrackStoppedCallback(
            tracks[trackIndex].get(),
            [](void* userdata, MIX_Track* track)
            {
                auto* self = static_cast<AudioService*>(userdata);
                for (unsigned int i = 0; i < self->tracks.size(); ++i)
                {
                    if (self->tracks[i].get() == track)
                    {
                        self->channelFinished.next(static_cast<int>(i));
                        break;
                    }
                }
            },
            this);
    }

    int AudioService::findFreeTrack()
    {
        // Search unreserved tracks for a free one
        for (unsigned int i = reservedCount; i < tracks.size(); ++i)
        {
            if (!sdlMixerContext->trackPlaying(tracks[i].get()))
            {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    AudioService::LoopToken AudioService::loopSound(const SoundHandle& sound)
    {
        int channel = findFreeTrack();
        if (channel == -1)
        {
            return LoopToken();
        }

        auto* track = tracks[channel].get();
        sdlMixerContext->setTrackAudio(track, sound.get());
        sdlMixerContext->setTrackGain(track, defaultGain);

        auto props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        sdlMixerContext->playTrack(track, props);
        SDL_DestroyProperties(props);

        return LoopToken(this, channel, sound);
    }

    int AudioService::playSound(const SoundHandle& sound)
    {
        int channel = findFreeTrack();
        if (channel == -1)
        {
            return -1;
        }

        auto* track = tracks[channel].get();
        sdlMixerContext->setTrackAudio(track, sound.get());
        sdlMixerContext->setTrackGain(track, defaultGain);
        sdlMixerContext->playTrack(track);

        return channel;
    }

    std::optional<AudioService::SoundHandle> AudioService::loadSound(const std::string& soundName)
    {
        auto soundIter = soundBank.find(soundName);
        if (soundIter != soundBank.end())
        {
            return soundIter->second;
        }

        auto bytes = fileSystem->readFile("sounds/" + soundName + ".WAV");
        if (!bytes)
        {
            return std::nullopt;
        }

        auto rwOps = sdlContext->rwFromConstMem(bytes->data(), bytes->size());
        auto audio = sdlMixerContext->loadAudioIO(rwOps.get(), true, false);
        if (!audio)
        {
            return std::nullopt;
        }

        std::shared_ptr<MIX_Audio> sound(audio.release(), [](MIX_Audio* a) { MIX_DestroyAudio(a); });
        soundBank[soundName] = sound;

        return sound;
    }

    void AudioService::reserveChannels(unsigned int count)
    {
        if (count > tracks.size())
        {
            allocateTracks(count);
        }
        reservedCount = count;
    }

    void AudioService::haltChannel(int channel)
    {
        if (channel >= 0 && static_cast<unsigned int>(channel) < tracks.size())
        {
            sdlMixerContext->stopTrack(tracks[channel].get());
        }
    }

    void AudioService::playSoundIfFree(const AudioService::SoundHandle& sound, unsigned int channel)
    {
        if (channel >= tracks.size())
        {
            return;
        }

        if (sdlMixerContext->trackPlaying(tracks[channel].get()))
        {
            return;
        }

        auto* track = tracks[channel].get();
        sdlMixerContext->setTrackAudio(track, sound.get());
        sdlMixerContext->setTrackGain(track, defaultGain);
        sdlMixerContext->playTrack(track);
    }

    Observable<int>& AudioService::getChannelFinished()
    {
        return channelFinished;
    }

    void AudioService::setVolume(int channel, int volume)
    {
        if (channel >= 0 && static_cast<unsigned int>(channel) < tracks.size())
        {
            // Old scale: 0-128 (MIX_MAX_VOLUME). New scale: 0.0-1.0
            float gain = static_cast<float>(volume) / 128.0f;
            sdlMixerContext->setTrackGain(tracks[channel].get(), gain);
        }
    }

    AudioService::LoopToken::LoopToken(AudioService* audioService, int channel, const AudioService::SoundHandle& sound)
        : audioService(audioService), channel(channel), sound(sound)
    {
    }

    AudioService::LoopToken::~LoopToken()
    {
        if (channel != -1)
        {
            audioService->haltChannel(channel);
        }
    }

    AudioService::LoopToken& AudioService::LoopToken::operator=(AudioService::LoopToken&& other) noexcept
    {
        audioService = other.audioService;
        channel = other.channel;
        sound = std::move(other.sound);

        other.channel = -1;

        return *this;
    }

    AudioService::LoopToken::LoopToken(AudioService::LoopToken&& other) noexcept
        : audioService(other.audioService), channel(other.channel), sound(std::move(other.sound))
    {
        other.channel = -1;
    }

    AudioService::LoopToken::LoopToken() : audioService(nullptr), channel(-1), sound(nullptr) {}

    std::optional<std::reference_wrapper<const AudioService::SoundHandle>> AudioService::LoopToken::getSound()
    {
        if (channel == -1)
        {
            return std::nullopt;
        }

        return sound;
    }
}
