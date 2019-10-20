#pragma once

#include <rwe/GlMesh.h>
#include <rwe/TextureHandle.h>
#include <rwe/VaoHandle.h>
#include <rwe/VboHandle.h>

namespace rwe
{
    struct ShaderMesh
    {
        SharedTextureHandle texture;
        GlMesh texturedVertices;

        ShaderMesh(
            const SharedTextureHandle& texture,
            GlMesh&& texturedVertices);
    };
}
