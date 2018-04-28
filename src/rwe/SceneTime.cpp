#include "SceneTime.h"

namespace rwe
{
    SceneTime nextSceneTime(SceneTime time)
    {
        return SceneTime(time.value + 1);
    }
}
