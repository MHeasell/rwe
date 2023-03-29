#pragma once

#include <rwe/sim/SimScalar.h>
#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<unsigned int, GameTimeTag>;

    GameTime deltaSecondsToTicks(float seconds);
    GameTime deltaSecondsToTicks(SimScalar seconds);
}
