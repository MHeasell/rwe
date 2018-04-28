#include "GameTime.h"

namespace rwe
{
    GameTime nextGameTime(GameTime time)
    {
        return GameTime(time.value + 1);
    }

    GameTimeDelta deltaSecondsToTicks(float seconds)
    {
        return GameTimeDelta(seconds * 60.0f);
    }
}
