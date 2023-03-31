#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct PlayerColorIndexTag;
    struct PlayerColorIndex : public OpaqueId<unsigned int, PlayerColorIndexTag>
    {
        explicit PlayerColorIndex(unsigned int value);
    };
}
