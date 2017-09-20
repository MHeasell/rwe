#include "Sprite.h"

namespace rwe
{
    Sprite::Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture)
        : bounds(bounds), texture(texture, Rectangle2f::fromTopLeft(0.0f, 0.0f, 1.0f, 1.0f)) {}

    Sprite::Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture, const Rectangle2f& textureRegion)
        : bounds(bounds), texture(texture, textureRegion) {}

    Sprite::Sprite(const Rectangle2f& bounds, const TextureRegion& texture)
        : bounds(bounds), texture(texture)
    {
    }

    bool Sprite::operator==(const Sprite& rhs) const
    {
        return bounds == rhs.bounds
            && texture == rhs.texture;
    }

    bool Sprite::operator!=(const Sprite& rhs) const
    {
        return !(rhs == *this);
    }
}
