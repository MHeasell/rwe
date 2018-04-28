#ifndef RWE_SCENETIME_H
#define RWE_SCENETIME_H

#include "OpaqueId.h"

namespace rwe
{
    struct SceneTimeTag;
    using SceneTime = OpaqueId<unsigned int, SceneTimeTag>;

    SceneTime nextSceneTime(SceneTime time);

    bool operator<(SceneTime a, SceneTime b);
    bool operator>(SceneTime a, SceneTime b);
    bool operator<=(SceneTime a, SceneTime b);
    bool operator>=(SceneTime a, SceneTime b);

    struct SceneTimeDeltaTag;
    using SceneTimeDelta = OpaqueId<unsigned int, SceneTimeDeltaTag>;

    bool operator<(SceneTimeDelta a, SceneTimeDelta b);
    bool operator>(SceneTimeDelta a, SceneTimeDelta b);
    bool operator<=(SceneTimeDelta a, SceneTimeDelta b);
    bool operator>=(SceneTimeDelta a, SceneTimeDelta b);

    SceneTime operator+(SceneTime a, SceneTimeDelta b);
    SceneTime operator-(SceneTime a, SceneTimeDelta b);

    SceneTimeDelta operator-(SceneTime a, SceneTime b);
}

#endif
