#pragma once

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct SceneTimeTag;
    using SceneTime = OpaqueUnit<unsigned int, SceneTimeTag>;
}
