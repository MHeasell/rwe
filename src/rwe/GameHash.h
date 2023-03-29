#pragma once

#include <cstdint>
#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct GameHashTag;
    using GameHash = OpaqueId<uint32_t, GameHashTag>;

    GameHash operator+(GameHash a, GameHash b);
    GameHash& operator+=(GameHash& a, const GameHash& b);
}
