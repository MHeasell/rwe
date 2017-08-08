#include "TextureHandle.h"

namespace rwe
{
    TextureHandle::TextureHandle(GLuint texture) : texture(texture) {}

    TextureHandle::TextureHandle(TextureHandle&& that) noexcept : texture(that.texture)
    {
        that.texture = 0;
    }

    TextureHandle& TextureHandle::operator=(TextureHandle&& that) noexcept
    {
        destroy();
        texture = that.texture;
        that.texture = 0;
        return *this;
    }

    TextureHandle::~TextureHandle()
    {
        if (isValid())
        {
            glDeleteTextures(1, &texture);
        }
    }

    TextureHandle::operator bool() const
    {
        return isValid();
    }

    GLuint TextureHandle::get() const
    {
        return texture;
    }

    bool TextureHandle::isValid() const
    {
        return texture != 0;
    }

    void TextureHandle::reset(GLuint newTexture)
    {
        destroy();
        texture = newTexture;
    }

    void TextureHandle::reset()
    {
        reset(0);
    }

    void TextureHandle::destroy()
    {
        glDeleteTextures(1, &texture);
    }
}
