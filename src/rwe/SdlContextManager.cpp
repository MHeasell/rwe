#include <stdexcept>
#include <SDL.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include "SdlContextManager.h"

namespace rwe
{
    SDLException::SDLException(const char* sdlError) : runtime_error(sdlError) {}

    SDLNetException::SDLNetException(const char* sdlError) : runtime_error(sdlError) {}

    SDLMixerException::SDLMixerException(const char* sdlError) : runtime_error(sdlError) {}

    SDLImageException::SDLImageException(const char* sdlError) : runtime_error(sdlError) {}

    SdlContextManager::SdlContextManager() {}

    const SdlContext* SdlContextManager::getSdlContext() const
    {
        return &sdlContext;
    }

    const SdlNetContext* SdlContextManager::getSdlNetContext() const
    {
        return &sdlNetContext;
    }

    const SdlMixerContext* SdlContextManager::getSdlMixerContext() const
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

    SdlNetContext::SdlNetContext()
    {
        if (SDLNet_Init() != 0)
        {
            throw SDLNetException(SDLNet_GetError());
        }
    }

    SdlNetContext::~SdlNetContext()
    {
        SDLNet_Quit();
    }

    SdlMixerContext::SdlMixerContext()
    {
        int flags = MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG;
        if ((Mix_Init(flags) & flags) != flags)
        {
            throw SDLMixerException(Mix_GetError());
        }
    }

    SdlMixerContext::~SdlMixerContext()
    {
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
