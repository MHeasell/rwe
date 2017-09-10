#ifndef RWE_SPRITE_H
#define RWE_SPRITE_H

#include <rwe/geometry/Rectangle2f.h>
#include <rwe/SharedTextureHandle.h>

namespace rwe
{
    struct Sprite
    {
        Rectangle2f bounds;
        SharedTextureHandle texture;
        Rectangle2f textureRegion;

        Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture);
        Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture, const Rectangle2f& textureRegion);

        bool operator==(const Sprite& rhs) const;

        bool operator!=(const Sprite& rhs) const;
    };
}


#endif
