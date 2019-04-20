#include "GameTime.h"

namespace rwe
{
    GameTime deltaSecondsToTicks(float seconds)
    {
        return GameTime(seconds * 60.0f);
    }
}
