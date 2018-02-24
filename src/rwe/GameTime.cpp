#include "GameTime.h"

namespace rwe
{
    GameTime nextGameTime(GameTime time)
    {
        return GameTime(time.value + 1);
    }

    bool operator<(GameTime a, GameTime b)
    {
        return a.value < b.value;
    }

    bool operator>(GameTime a, GameTime b)
    {
        return a.value > b.value;
    }

    bool operator<=(GameTime a, GameTime b)
    {
        return a.value <= b.value;
    }

    bool operator>=(GameTime a, GameTime b)
    {
        return a.value >= b.value;
    }

    bool operator<(GameTimeDelta a, GameTimeDelta b)
    {
        return a.value < b.value;
    }

    bool operator>(GameTimeDelta a, GameTimeDelta b)
    {
        return a.value > b.value;
    }

    bool operator<=(GameTimeDelta a, GameTimeDelta b)
    {
        return a.value <= b.value;
    }

    bool operator>=(GameTimeDelta a, GameTimeDelta b)
    {
        return a.value >= b.value;
    }

    GameTime operator+(GameTime a, GameTimeDelta b)
    {
        return GameTime(a.value + b.value);
    }

    GameTime operator-(GameTime a, GameTimeDelta b)
    {
        return GameTime(a.value - b.value);
    }

    GameTimeDelta operator-(GameTime a, GameTime b)
    {
        return GameTimeDelta(a.value - b.value);
    }

    GameTimeDelta deltaSecondsToTicks(float seconds)
    {
        return GameTimeDelta(seconds * 60.0f);
    }
}
