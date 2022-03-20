#pragma once

#include <memory>
#include <rwe/math/Vector3f.h>
#include <rwe/render/ShaderMesh.h>

namespace rwe
{
    struct UnitPieceMeshInfo
    {
        std::shared_ptr<ShaderMesh> mesh;

        // Used for vector-based SFX.
        Vector3f firstVertexPosition;
        Vector3f secondVertexPosition;
    };
}
