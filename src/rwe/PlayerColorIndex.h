#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct PlayerColorIndexTag;
    struct PlayerColorIndex : public OpaqueId<int, PlayerColorIndexTag>
    {
        explicit PlayerColorIndex(int value);
    };
}
