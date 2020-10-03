#pragma once

#include <rwe/OpaqueId.h>
#include <rwe/SceneManager.h>
#include <rwe/sim/GameTime.h>

namespace rwe
{
    struct CobSleepDurationTag;
    struct CobSleepDuration : public OpaqueId<uint32_t, CobSleepDurationTag>
    {
        CobSleepDuration() = default;
        explicit CobSleepDuration(ValueType value) : OpaqueId(value) {}

        GameTime toGameTime() const
        {
            return GameTime(value / SceneManager::TickInterval);
        }
    };
}
