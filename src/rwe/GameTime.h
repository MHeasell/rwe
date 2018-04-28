#ifndef RWE_GAMETIME_H
#define RWE_GAMETIME_H

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<unsigned int, GameTimeTag>;
    using GameTimeDelta = OpaqueUnitDelta<unsigned int, GameTimeTag>;

    GameTime nextGameTime(GameTime time);

    GameTimeDelta deltaSecondsToTicks(float seconds);
}

#endif
