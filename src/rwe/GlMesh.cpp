#include "GlMesh.h"

namespace rwe
{
    GlMesh::GlMesh(
        VaoHandle&& vao,
        VboHandle&& vbo,
        unsigned int vertexCount)
        : vao(std::move(vao)),
          vbo(std::move(vbo)),
          vertexCount(vertexCount)
    {
    }
}
