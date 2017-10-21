#ifndef RWE_SPRITE_H
#define RWE_SPRITE_H

#include <rwe/TextureHandle.h>
#include <rwe/TextureRegion.h>
#include <rwe/geometry/Rectangle2f.h>

namespace rwe
{
    struct Sprite
    {
        Rectangle2f bounds;
        TextureRegion texture;

        Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture);
        Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture, const Rectangle2f& textureRegion);
        Sprite(const Rectangle2f& bounds, const TextureRegion& texture);

        bool operator==(const Sprite& rhs) const;

        bool operator!=(const Sprite& rhs) const;
    };
}


#endif
