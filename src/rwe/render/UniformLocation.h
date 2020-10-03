#pragma once

#include <GL/glew.h>
#include <rwe/OpaqueId.h>

namespace rwe
{
    struct UniformLocationTag;
    using UniformLocation = OpaqueId<GLint, UniformLocationTag>;
}
