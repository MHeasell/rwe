#ifndef RWE_TEXTUREHANDLE_H
#define RWE_TEXTUREHANDLE_H

#include <GL/glew.h>
#include <rwe/SharedHandle.h>
#include <rwe/GlIdentifier.h>

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

#endif
