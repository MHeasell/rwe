#ifndef RWE_DEBUGLINESMESH_H
#define RWE_DEBUGLINESMESH_H

#include "VaoHandle.h"
#include "VboHandle.h"

namespace rwe
{
    struct DebugLinesMesh
    {
        VaoHandle vao;
        VboHandle vbo;
        unsigned int vertexCount;
    };
}

#endif
