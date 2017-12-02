#ifndef RWE_GLTEXTUREDMESH_H
#define RWE_GLTEXTUREDMESH_H

#include "GlMesh.h"
#include "TextureHandle.h"

namespace rwe
{
    struct GlTexturedMesh
    {
        SharedTextureHandle texture;
        GlMesh mesh;

        GlTexturedMesh(const SharedTextureHandle& texture, GlMesh&& mesh);
    };
}

#endif
