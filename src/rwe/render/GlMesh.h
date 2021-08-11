#pragma once

#include <rwe/render/VaoHandle.h>
#include <rwe/render/VboHandle.h>

namespace rwe
{
    struct GlMesh
    {
        VaoHandle vao;
        VboHandle vbo;
        int vertexCount;

        GlMesh(VaoHandle&& vao, VboHandle&& vbo, int vertexCount);
    };
}
