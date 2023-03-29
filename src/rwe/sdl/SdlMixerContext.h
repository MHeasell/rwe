#pragma once

#include <SDL_mixer.h>
#include <functional>
#include <memory>
#include <rwe/sdl/SdlMixerException.h>

namespace rwe
{
    class SdlMixerContext
    {
    private:
        SdlMixerContext()
        {
            int flags = 0;
            if ((Mix_Init(flags) & flags) != flags)
            {
                throw SDLMixerException(Mix_GetError());
            }

            if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
            {
                throw SDLMixerException(Mix_GetError());
            }
        }
        SdlMixerContext(const SdlMixerContext&) = delete;
        ~SdlMixerContext()
        {
            Mix_CloseAudio();
            Mix_Quit();
        }

        friend class SdlContextManager;

    public:
        struct MixChunkDeleter
        {
            void operator()(Mix_Chunk* chunk) { Mix_FreeChunk(chunk); }
        };

        std::unique_ptr<Mix_Chunk, MixChunkDeleter> loadWavRw(SDL_RWops* src)
        {
            return std::unique_ptr<Mix_Chunk, MixChunkDeleter>(Mix_LoadWAV_RW(src, 0));
        };

        void allocateChannels(int numChans)
        {
            Mix_AllocateChannels(numChans);
        }

        int playChannel(int channel, Mix_Chunk* chunk, int loops)
        {
            return Mix_PlayChannel(channel, chunk, loops);
        }

        void haltChannel(int channel)
        {
            Mix_HaltChannel(channel);
        }

        int volumeChunk(Mix_Chunk* chunk, int volume)
        {
            return Mix_VolumeChunk(chunk, volume);
        }

        int reserveChannels(int num)
        {
            return Mix_ReserveChannels(num);
        }

        int playing(int channel)
        {
            return Mix_Playing(channel);
        }

        void channelFinished(std::function<void(int)> f)
        {
            static auto cachedF = f;
            void (*callback)(int) = [](int channel) { cachedF(channel); };
            return Mix_ChannelFinished(callback);
        }

        int volume(int channel, int volume)
        {
            return Mix_Volume(channel, volume);
        }
    };
}
