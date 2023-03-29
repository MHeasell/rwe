#include "SdlContextManager.h"

namespace rwe
{
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

}
