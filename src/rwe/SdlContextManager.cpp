#include "SdlContextManager.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdexcept>

namespace rwe
{
    SDLException::SDLException(const char* sdlError) : runtime_error(sdlError) {}

    SDLMixerException::SDLMixerException(const char* sdlError) : runtime_error(sdlError) {}

    SDLImageException::SDLImageException(const char* sdlError) : runtime_error(sdlError) {}

    SdlContextManager::SdlContextManager() {}

    const SdlContext* SdlContextManager::getSdlContext() const
    {
        return &sdlContext;
    }

    SdlContext* SdlContextManager::getSdlContext()
    {
        return &sdlContext;
    }

    SdlMixerContext* SdlContextManager::getSdlMixerContext()
    {
        return &sdlMixerContext;
    }

    const SdlImageContext* SdlContextManager::getSdlImageContext() const
    {
        return &sdlImageContext;
    }

    SdlContext::SdlContext()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
        {
            throw SDLException(SDL_GetError());
        }
    }

    SdlContext::~SdlContext()
    {
        SDL_Quit();
    }

    SdlMixerContext::SdlMixerContext()
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

    SdlMixerContext::~SdlMixerContext()
    {
        Mix_CloseAudio();
        Mix_Quit();
    }

    SdlImageContext::SdlImageContext()
    {
        int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
        if ((IMG_Init(flags) & flags) != flags)
        {
            throw SDLImageException(IMG_GetError());
        }
    }

    SdlImageContext::~SdlImageContext()
    {
        IMG_Quit();
    }
}
