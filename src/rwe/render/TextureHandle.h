#pragma once

#include <GL/glew.h>
#include <rwe/SharedHandle.h>
#include <rwe/render/GlIdentifier.h>

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
