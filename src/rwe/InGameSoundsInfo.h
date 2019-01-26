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

        std::optional<AudioService::SoundHandle> nextBuildMenu;

        std::optional<AudioService::SoundHandle> buildButton;
        std::optional<AudioService::SoundHandle> ordersButton;

        std::optional<AudioService::SoundHandle> addBuild;
        std::optional<AudioService::SoundHandle> okToBuild;
    };
}

#endif
