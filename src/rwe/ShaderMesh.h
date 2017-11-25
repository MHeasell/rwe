#ifndef RWE_SHADERMESH_H
#define RWE_SHADERMESH_H

#include "TextureHandle.h"
#include "VaoHandle.h"
#include "VboHandle.h"
#include "GlMesh.h"

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
