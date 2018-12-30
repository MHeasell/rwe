#ifndef RWE_INGAMESOUNDSINFO_H
#define RWE_INGAMESOUNDSINFO_H

#include <rwe/AudioService.h>

namespace rwe
{
    struct InGameSoundsInfo
    {
        std::optional<AudioService::SoundHandle> immediateOrders;
        std::optional<AudioService::SoundHandle> specialOrders;
        std::optional<AudioService::SoundHandle> setFireOrders;
    };
}

#endif
