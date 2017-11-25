#ifndef RWE_SHADERMESH_H
#define RWE_SHADERMESH_H

#include "TextureHandle.h"
#include "VaoHandle.h"
#include "VboHandle.h"

namespace rwe
{
    struct ShaderMesh
    {
        SharedTextureHandle texture;

        VboHandle texturedVertices;
        unsigned int texturedVerticesCount;
        VaoHandle texturedVerticesVao;
        VboHandle coloredVertices;
        unsigned int coloredVerticesCount;
        VaoHandle coloredVerticesVao;

        ShaderMesh(
            const SharedTextureHandle& texture,
            VboHandle&& texturedVertices,
            unsigned int texturedVerticesCount,
            VaoHandle&& texturedVerticesVao,
            VboHandle&& coloredVertices,
            unsigned int coloredVerticesCount,
            VaoHandle&& coloredVerticesVao);
    };
}

#endif
