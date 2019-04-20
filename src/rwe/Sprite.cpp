#include "Sprite.h"

namespace rwe
{
    Sprite::Sprite(const Rectangle2f& bounds, SharedTextureHandle texture, std::shared_ptr<GlMesh> mesh)
        : bounds(bounds), texture(std::move(texture)), mesh(std::move(mesh))
    {
    }

    Matrix4f Sprite::getTransform() const
    {
        return Matrix4f::translation(Vector3f(bounds.position.x, bounds.position.y, 0.0f))
            * Matrix4f::scale(Vector3f(bounds.extents.x, bounds.extents.y, 1.0f));
    }
}
