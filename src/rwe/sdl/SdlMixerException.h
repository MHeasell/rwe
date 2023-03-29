#pragma once

#include <stdexcept>

namespace rwe
{
    class SDLMixerException : public std::runtime_error
    {
    public:
        explicit SDLMixerException(const char* sdlError) : std::runtime_error(sdlError) {}
    };
}
