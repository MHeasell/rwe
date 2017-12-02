#include "ShaderMesh.h"

namespace rwe
{
    ShaderMesh::ShaderMesh(
        const SharedTextureHandle& texture,
        GlMesh&& texturedVertices,
        GlMesh&& coloredVertices)
        : texture(texture),
          texturedVertices(std::move(texturedVertices)),
          coloredVertices(std::move(coloredVertices))
    {
    }
}
