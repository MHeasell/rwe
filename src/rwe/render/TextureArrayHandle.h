#pragma once

#include <GL/glew.h>
#include <rwe/render/GlIdentifier.h>
#include <rwe/util/SharedHandle.h>

namespace rwe
{
    struct TextureArrayIdTag;
    using TextureArrayIdentifier = GlIdentifier<TextureArrayIdTag>;

    struct TextureArrayHandleDeleter
    {
        void operator()(TextureArrayIdentifier handle)
        {
            if (handle.isValid())
            {
                glDeleteTextures(1, &(handle.value));
            }
        }
    };

    using TextureArrayHandle = UniqueHandle<TextureArrayIdentifier, TextureArrayHandleDeleter>;
    using SharedTextureArrayHandle = SharedHandle<TextureArrayIdentifier, TextureArrayHandleDeleter>;
}
