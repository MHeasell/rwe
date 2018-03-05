#ifndef RWE_SHADERMESH_H
#define RWE_SHADERMESH_H

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
        GlMesh coloredVertices;

        ShaderMesh(
            const SharedTextureHandle& texture,
            GlMesh&& texturedVertices,
            GlMesh&& coloredVertices);
    };
}

#endif
