#ifndef RWE_UNIFORMLOCATION_H
#define RWE_UNIFORMLOCATION_H

#include <rwe/OpaqueId.h>
#include <GL/glew.h>

namespace rwe
{
    struct UniformLocationTag;
    using UniformLocation = OpaqueId<GLint, UniformLocationTag>;
}

#endif
