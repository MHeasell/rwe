#ifndef RWE_SHAREDTEXTUREHANDLE_H
#define RWE_SHAREDTEXTUREHANDLE_H

#include <functional>
#include <GL/glew.h>
#include <rwe/SharedHandle.h>

namespace rwe
{
    struct TextureIdentifier
    {
        GLuint value{0};

        TextureIdentifier() = default;
        explicit TextureIdentifier(GLuint value) : value(value)
        {
        }

        bool isValid() const
        {
            return value != 0;
        }

        explicit operator bool() const
        {
            return isValid();
        }

        bool operator==(const TextureIdentifier& rhs) const
        {
            return value == rhs.value;
        }

        bool operator!=(const TextureIdentifier& rhs) const
        {
            return !(rhs == *this);
        }
    };
}

namespace std
{
    template <>
    struct hash<rwe::TextureIdentifier>
    {
        std::size_t operator()(const rwe::TextureIdentifier& f) const noexcept
        {
            return std::hash<GLuint>()(f.value);
        }
    };
}

namespace rwe
{
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
