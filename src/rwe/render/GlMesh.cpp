#include "GlMesh.h"

namespace rwe
{
    GlMesh::GlMesh(
        VaoHandle&& vao,
        VboHandle&& vbo,
        size_t vertexCount)
        : vao(std::move(vao)),
          vbo(std::move(vbo)),
          vertexCount(static_cast<int>(vertexCount))
    {
    }
}
