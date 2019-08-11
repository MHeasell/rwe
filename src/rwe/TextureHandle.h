#pragma once

#include <GL/glew.h>
#include <rwe/GlIdentifier.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct TextureIdTag;
    using TextureIdentifier = GlIdentifier<TextureIdTag>;

    struct TextureHandleDeleter
    {
        void operator()(TextureIdentifier handle)
        {
            if (handle.isValid())
            {
                glDeleteTextures(1, &(handle.value));
            }
        }
    };

    using TextureHandle = UniqueHandle<TextureIdentifier, TextureHandleDeleter>;
    using SharedTextureHandle = SharedHandle<TextureIdentifier, TextureHandleDeleter>;
}
