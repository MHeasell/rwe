#include "Sprite.h"

namespace rwe
{
    Sprite::Sprite(const Rectangle2f& bounds, GlTexturedMesh&& mesh) : bounds(bounds), mesh(std::move(mesh))
    {
    }
}
