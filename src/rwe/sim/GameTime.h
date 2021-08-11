#pragma once

#include <rwe/OpaqueUnit.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<int, GameTimeTag>;

    GameTime deltaSecondsToTicks(float seconds);
    GameTime deltaSecondsToTicks(SimScalar seconds);
}
