#include "TextureRegion.h"

namespace rwe
{
    TextureRegion::TextureRegion(const SharedTextureHandle& texture, const Rectangle2f& region)
        : texture(texture), region(region)
    {
    }

    bool TextureRegion::operator==(const TextureRegion& rhs) const
    {
        return texture == rhs.texture && region == rhs.region;
    }

    bool TextureRegion::operator!=(const TextureRegion& rhs) const
    {
        return !(rhs == *this);
    }
}
