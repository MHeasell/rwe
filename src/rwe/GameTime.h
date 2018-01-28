#ifndef RWE_GAMETIME_H
#define RWE_GAMETIME_H

#include "OpaqueId.h"

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueId<unsigned int, GameTimeTag>;

    GameTime nextGameTime(GameTime time);

    bool operator<(GameTime a, GameTime b);
    bool operator>(GameTime a, GameTime b);
    bool operator<=(GameTime a, GameTime b);
    bool operator>=(GameTime a, GameTime b);

    struct GameTimeDeltaTag;
    using GameTimeDelta = OpaqueId<unsigned int, GameTimeDeltaTag>;

    GameTime operator+(GameTime a, GameTimeDelta b);
    GameTime operator-(GameTime a, GameTimeDelta b);
}

#endif
