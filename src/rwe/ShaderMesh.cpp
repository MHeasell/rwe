#include "ShaderMesh.h"

namespace rwe
{
    ShaderMesh::ShaderMesh(
        GlMesh&& vertices,
        GlMesh&& teamVertices)
        : vertices(std::move(vertices)),
          teamVertices(std::move(teamVertices))
    {
    }
}
