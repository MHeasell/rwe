#pragma once

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<unsigned int, GameTimeTag>;

    GameTime deltaSecondsToTicks(float seconds);
}
