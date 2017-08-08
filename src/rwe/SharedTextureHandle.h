#ifndef RWE_SHAREDTEXTUREHANDLE_H
#define RWE_SHAREDTEXTUREHANDLE_H

#include <GL/glew.h>

namespace rwe
{
    class SharedTextureHandle
    {
    private:
        GLuint handle;
        unsigned int* referenceCount;

    public:

        explicit SharedTextureHandle(GLuint handle);
        ~SharedTextureHandle();

        SharedTextureHandle(const SharedTextureHandle& other);

        SharedTextureHandle& operator=(const SharedTextureHandle& other);

        SharedTextureHandle(SharedTextureHandle&& other) noexcept;

        SharedTextureHandle& operator=(SharedTextureHandle&& other) noexcept;

        /**
		 * Equivalent to isValid()
		 */
        explicit operator bool() const;

        /** Returns the underlying texture handle. */
        GLuint get() const;

        /**
		 * Returns true if the handle contains a valid texture, otherwise false.
		 */
        bool isValid() const;

        unsigned int useCount() const;

        /** Replaces the contents of the handle with the given texture. */
        void reset(GLuint newTexture);

        /** Resets the handle to the null texture. */
        void reset();

    private:
        void destroy();
    };
}

#endif
