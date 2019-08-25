#pragma once

#include <rwe/OpaqueUnit.h>
#include <rwe/SimScalar.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<unsigned int, GameTimeTag>;

    GameTime deltaSecondsToTicks(float seconds);
    GameTime deltaSecondsToTicks(SimScalar seconds);
}
