#include "SceneTime.h"

namespace rwe
{
    SceneTime nextSceneTime(SceneTime time)
    {
        return SceneTime(time.value + 1);
    }

    bool operator<(SceneTime a, SceneTime b)
    {
        return a.value < b.value;
    }

    bool operator>(SceneTime a, SceneTime b)
    {
        return a.value > b.value;
    }

    bool operator<=(SceneTime a, SceneTime b)
    {
        return a.value <= b.value;
    }

    bool operator>=(SceneTime a, SceneTime b)
    {
        return a.value >= b.value;
    }

    bool operator<(SceneTimeDelta a, SceneTimeDelta b)
    {
        return a.value < b.value;
    }

    bool operator>(SceneTimeDelta a, SceneTimeDelta b)
    {
        return a.value > b.value;
    }

    bool operator<=(SceneTimeDelta a, SceneTimeDelta b)
    {
        return a.value <= b.value;
    }

    bool operator>=(SceneTimeDelta a, SceneTimeDelta b)
    {
        return a.value >= b.value;
    }

    SceneTime operator+(SceneTime a, SceneTimeDelta b)
    {
        return SceneTime(a.value + b.value);
    }

    SceneTime operator-(SceneTime a, SceneTimeDelta b)
    {
        return SceneTime(a.value - b.value);
    }

    SceneTimeDelta operator-(SceneTime a, SceneTime b)
    {
        return SceneTimeDelta(a.value - b.value);
    }
}
