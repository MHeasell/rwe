#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct PlayerColorIndexTag;
    struct PlayerColorIndex : public OpaqueId<unsigned int, PlayerColorIndexTag>
    {
        explicit PlayerColorIndex(unsigned int value);
    };
}
