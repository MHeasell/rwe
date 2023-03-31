#pragma once

#include <rwe/util/OpaqueUnit.h>

namespace rwe
{
    struct SceneTimeTag;
    using SceneTime = OpaqueUnit<unsigned int, SceneTimeTag>;
}
