#include "GraphicsContext.h"

#include <GL/glew.h>

namespace rwe
{
    void GraphicsContext::drawTriangle(const Vector3f& a, const Vector3f& b, const Vector3f& c)
    {
        glBegin(GL_TRIANGLES);
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
        glEnd();
    }

    void GraphicsContext::setBackgroundColor(float r, float g, float b)
    {
        glClearColor(r, g, b, 1.0f);
        assert(glGetError() == GL_NO_ERROR);
    }

    void GraphicsContext::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, GLuint texture)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(x, y);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(x, y + height);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(x + width, y + height);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(x + width, y);

        glEnd();
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture)
    {
        drawTexture(x, y, width, height, texture.get());
    }

    void GraphicsContext::drawSprite(float x, float y, const Sprite& sprite)
    {
        drawTexture(
            x + sprite.bounds.left(),
            y + sprite.bounds.top(),
            sprite.bounds.width(),
            sprite.bounds.height(),
            sprite.texture);
    }
}
