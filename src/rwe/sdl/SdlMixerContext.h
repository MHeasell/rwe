#pragma once

#include <SDL3_mixer/SDL_mixer.h>
#include <functional>
#include <memory>
#include <rwe/sdl/SdlMixerException.h>

namespace rwe
{
    class SdlMixerContext
    {
    public:
        struct AudioDeleter
        {
            void operator()(MIX_Audio* audio) { MIX_DestroyAudio(audio); }
        };

        struct TrackDeleter
        {
            void operator()(MIX_Track* track) { MIX_DestroyTrack(track); }
        };

        using AudioPtr = std::unique_ptr<MIX_Audio, AudioDeleter>;
        using TrackPtr = std::unique_ptr<MIX_Track, TrackDeleter>;

    private:
        MIX_Mixer* mixer;

        SdlMixerContext()
        {
            if (!MIX_Init())
            {
                throw SDLMixerException("MIX_Init failed");
            }

            mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            if (!mixer)
            {
                MIX_Quit();
                throw SDLMixerException(SDL_GetError());
            }
        }

        SdlMixerContext(const SdlMixerContext&) = delete;

        ~SdlMixerContext()
        {
            MIX_DestroyMixer(mixer);
            MIX_Quit();
        }

        friend class SdlContextManager;

    public:
        MIX_Mixer* getMixer() { return mixer; }

        AudioPtr loadAudioIO(SDL_IOStream* io, bool predecode, bool closeio)
        {
            return AudioPtr(MIX_LoadAudio_IO(mixer, io, predecode, closeio));
        }

        TrackPtr createTrack()
        {
            return TrackPtr(MIX_CreateTrack(mixer));
        }

        bool setTrackAudio(MIX_Track* track, MIX_Audio* audio)
        {
            return MIX_SetTrackAudio(track, audio);
        }

        bool playTrack(MIX_Track* track, SDL_PropertiesID options = 0)
        {
            return MIX_PlayTrack(track, options);
        }

        bool stopTrack(MIX_Track* track, Sint64 fadeOutFrames = 0)
        {
            return MIX_StopTrack(track, fadeOutFrames);
        }

        bool trackPlaying(MIX_Track* track)
        {
            return MIX_TrackPlaying(track);
        }

        bool setTrackGain(MIX_Track* track, float gain)
        {
            return MIX_SetTrackGain(track, gain);
        }

        bool setTrackStoppedCallback(MIX_Track* track, MIX_TrackStoppedCallback cb, void* userdata)
        {
            return MIX_SetTrackStoppedCallback(track, cb, userdata);
        }

        bool playAudio(MIX_Audio* audio)
        {
            return MIX_PlayAudio(mixer, audio);
        }
    };
}
