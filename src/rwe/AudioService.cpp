#include "AudioService.h"
#include <cassert>

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
        this->sdlMixerContext->channelFinished(std::bind(&Subject<int>::next, &channelFinished, std::placeholders::_1));
    }

    AudioService::LoopToken AudioService::loopSound(const SoundHandle& sound)
    {
        auto channel = sdlMixerContext->playChannel(-1, sound.get(), -1);
        return LoopToken(this, channel, sound);
    }

    int AudioService::playSound(const SoundHandle& sound)
    {
        return sdlMixerContext->playChannel(-1, sound.get(), 0);
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
        std::shared_ptr<Mix_Chunk> sound(sdlMixerContext->loadWavRw(rwOps.get()));
        sdlMixerContext->volumeChunk(sound.get(), MIX_MAX_VOLUME / 4);
        soundBank[soundName] = sound;

        return sound;
    }

    void AudioService::reserveChannels(int count)
    {
        int num = sdlMixerContext->reserveChannels(count);
        assert(num >= 0);
        if (num < count)
        {
            throw std::runtime_error("Failed to reserve audio channels");
        }
    }

    void AudioService::haltChannel(int channel)
    {
        sdlMixerContext->haltChannel(channel);
    }

    void AudioService::playSoundIfFree(const AudioService::SoundHandle& sound, int channel)
    {
        if (sdlMixerContext->playing(channel))
        {
            return;
        }

        sdlMixerContext->playChannel(channel, sound.get(), 0);
    }

    Observable<int>& AudioService::getChannelFinished()
    {
        return channelFinished;
    }

    void AudioService::setVolume(int channel, int volume)
    {
        sdlMixerContext->volume(channel, volume);
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
