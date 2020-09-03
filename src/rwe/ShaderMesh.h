#pragma once

#include <rwe/GlMesh.h>
#include <rwe/TextureHandle.h>
#include <rwe/VaoHandle.h>
#include <rwe/VboHandle.h>

namespace rwe
{
    struct ShaderMesh
    {
        GlMesh vertices;
        GlMesh teamVertices;

        ShaderMesh(
            GlMesh&& vertices,
            GlMesh&& teamVertices);
    };
}
