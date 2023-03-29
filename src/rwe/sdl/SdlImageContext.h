#pragma once

#include <SDL_image.h>
#include <rwe/sdl/SdlImageException.h>

namespace rwe
{
    class SdlImageContext
    {
    private:
        SdlImageContext()
        {

            int flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
            if ((IMG_Init(flags) & flags) != flags)
            {
                throw rwe::SDLImageException(IMG_GetError());
            }
        }
        SdlImageContext(const SdlImageContext&) = delete;
        ~SdlImageContext()
        {

            IMG_Quit();
        }

        friend class SdlContextManager;
    };
}
