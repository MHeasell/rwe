#ifndef RWE_SCENETIME_H
#define RWE_SCENETIME_H

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct SceneTimeTag;
    using SceneTime = OpaqueUnit<unsigned int, SceneTimeTag>;
    using SceneTimeDelta = OpaqueUnitDelta<unsigned int, SceneTimeTag>;

    SceneTime nextSceneTime(SceneTime time);
}

#endif
