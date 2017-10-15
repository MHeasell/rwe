#ifndef RWE_SHAREDTEXTUREHANDLE_H
#define RWE_SHAREDTEXTUREHANDLE_H

#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct SharedTextureHandleDeleter
    {
        void operator()(GLuint handle)
        {
            glDeleteTextures(1, &handle);
        }
    };

    using SharedTextureHandle = SharedHandle<GLuint, SharedTextureHandleDeleter>;
}

#endif
