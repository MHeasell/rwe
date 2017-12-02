#ifndef RWE_UNIFORMLOCATION_H
#define RWE_UNIFORMLOCATION_H

#include <GL/glew.h>
#include <rwe/OpaqueId.h>

namespace rwe
{
    struct UniformLocationTag;
    using UniformLocation = OpaqueId<GLint, UniformLocationTag>;
}

#endif
