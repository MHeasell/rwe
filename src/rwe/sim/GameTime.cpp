#include "GameTime.h"

namespace rwe
{
    GameTime deltaSecondsToTicks(float seconds)
    {
        return GameTime(seconds * 30.0f);
    }

    GameTime deltaSecondsToTicks(SimScalar seconds)
    {
        return GameTime(seconds.value * 30.0f);
    }
}
