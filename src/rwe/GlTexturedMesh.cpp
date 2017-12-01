#include "GlTexturedMesh.h"

namespace rwe
{
    GlTexturedMesh::GlTexturedMesh(const SharedTextureHandle& texture, GlMesh&& mesh)
        : texture(texture),
          mesh(std::move(mesh))
    {
    }
}
