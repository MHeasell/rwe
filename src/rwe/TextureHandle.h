#ifndef RWE_TEXTUREHANDLE_H
#define RWE_TEXTUREHANDLE_H

#include <GL/glew.h>

namespace rwe
{
    /**
	 * Wraps an OpenGL texture handle.
	 * When this object is destroyed
	 * it will delete the texture in OpenGL.
	 */
    class TextureHandle
    {
    private:
        GLuint texture{0};

    public:
        TextureHandle() = default;
        explicit TextureHandle(GLuint texture);

        TextureHandle(TextureHandle&) = delete;
        TextureHandle& operator=(const TextureHandle&) = delete;

        TextureHandle(TextureHandle&& that) noexcept;

        TextureHandle& operator=(TextureHandle&& that) noexcept;

        ~TextureHandle();

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

        /** Replaces the contents of the handle with the given texture. */
        void reset(GLuint newTexture);

        /** Resets the handle to the null texture. */
        void reset();

    private:
        void destroy();
    };
}

#endif
