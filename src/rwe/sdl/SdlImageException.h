#pragma once

#include <stdexcept>

namespace rwe
{
    class SDLImageException : public std::runtime_error
    {
    public:
        explicit SDLImageException(const char* sdlError) : std::runtime_error(sdlError) {}
    };
}
