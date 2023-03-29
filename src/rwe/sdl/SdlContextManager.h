#pragma once

#include <rwe/sdl/SdlContext.h>
#include <rwe/sdl/SdlImageContext.h>
#include <rwe/sdl/SdlMixerContext.h>

namespace rwe
{
    /**
     * Manages the lifetime of SDL contexts.
     * Contexts accessed via this manager
     * are only valid for the lifetime of the manager.
     *
     * SDL is a global resource so don't instantiate more than one of these.
     */
    class SdlContextManager
    {
    public:
        SdlContextManager() = default;
        SdlContextManager(const SdlContextManager&) = delete;

        const rwe::SdlContext* getSdlContext() const;
        rwe::SdlContext* getSdlContext();
        rwe::SdlMixerContext* getSdlMixerContext();
        const SdlImageContext* getSdlImageContext() const;

    private:
        rwe::SdlContext sdlContext;
        rwe::SdlMixerContext sdlMixerContext;
        SdlImageContext sdlImageContext;
    };
}
