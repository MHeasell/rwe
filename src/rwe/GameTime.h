#ifndef RWE_GAMETIME_H
#define RWE_GAMETIME_H

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct GameTimeTag;
    using GameTime = OpaqueUnit<unsigned int, GameTimeTag>;

    GameTime deltaSecondsToTicks(float seconds);
}

#endif
