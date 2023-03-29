#pragma once

#include <stdexcept>

namespace rwe
{
    class SDLException : public std::runtime_error
    {
    public:
        explicit SDLException(const char* sdlError) : std::runtime_error(sdlError) {}
    };
}
