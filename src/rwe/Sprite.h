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

        explicit Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture);
    };
}


#endif
