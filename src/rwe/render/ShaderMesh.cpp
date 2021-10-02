#include "ShaderMesh.h"

namespace rwe
{
    ShaderMesh::ShaderMesh(
        std::optional<GlMesh>&& vertices,
        std::optional<GlMesh>&& teamVertices)
        : vertices(std::move(vertices)),
          teamVertices(std::move(teamVertices))
    {
    }
}
