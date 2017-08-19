#include "Sprite.h"

namespace rwe
{
    Sprite::Sprite(const Rectangle2f& bounds, const SharedTextureHandle& texture) : bounds(bounds), texture(texture) {}
}
