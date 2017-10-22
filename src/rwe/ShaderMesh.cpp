#include "ShaderMesh.h"

namespace rwe
{
    ShaderMesh::ShaderMesh(
        const SharedTextureHandle& texture,
        VboHandle&& texturedVertices,
        unsigned int texturedVerticesCount,
        VaoHandle&& texturedVerticesVao,
        VboHandle&& coloredVertices,
        unsigned int coloredVerticesCount,
        VaoHandle&& coloredVerticesVao)
        : texture(texture),
          texturedVertices(std::move(texturedVertices)),
          texturedVerticesCount(texturedVerticesCount),
          texturedVerticesVao(std::move(texturedVerticesVao)),
          coloredVertices(std::move(coloredVertices)),
          coloredVerticesCount(coloredVerticesCount),
          coloredVerticesVao(std::move(coloredVerticesVao))
    {
    }
}
