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

    GameTime operator+(GameTime a, GameTimeDelta b)
    {
        return GameTime(a.value + b.value);
    }

    GameTime operator-(GameTime a, GameTimeDelta b)
    {
        return GameTime(a.value - b.value);
    }
}
