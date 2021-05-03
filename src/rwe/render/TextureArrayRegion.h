#pragma once

#include <rwe/render/TextureArrayHandle.h>

namespace rwe
{
    struct TextureArrayRegion
    {
        SharedTextureArrayHandle texture;
        int index;

        TextureArrayRegion(const SharedTextureArrayHandle& texture, int index)
            : texture(texture), index(index) {}
    };
}
