#include "ShaderMesh.h"

namespace rwe
{
    ShaderMesh::ShaderMesh(
        const SharedTextureHandle& texture,
        GlMesh&& texturedVertices)
        : texture(texture),
          texturedVertices(std::move(texturedVertices))
    {
    }
}
