#pragma once

#include <rwe/render/GlMesh.h>
#include <rwe/render/TextureHandle.h>
#include <rwe/render/VaoHandle.h>
#include <rwe/render/VboHandle.h>

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
