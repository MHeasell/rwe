#include "GraphicsContext.h"
#include "rwe_string.h"

#include <GL/glew.h>

namespace rwe
{
    void requireNoOpenGlError()
    {
        auto error = glGetError();
        if (error != GL_NO_ERROR)
        {
            throw OpenGlException(error);
        }
    }

    GraphicsException::GraphicsException(const std::string& __arg) : runtime_error(__arg) {}

    GraphicsException::GraphicsException(const char* string) : runtime_error(string) {}

    OpenGlException::OpenGlException(GLenum error) : GraphicsException(std::string("OpenGL error: ") + std::to_string(error))
    {
    }

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

    void GraphicsContext::drawTextureRegion(float x, float y, float width, float height, const TextureRegion& texture)
    {
        drawTextureRegion(
            x,
            y,
            width,
            height,
            texture.texture,
            texture.region.left(),
            texture.region.top(),
            texture.region.width(),
            texture.region.height());
    }

    void GraphicsContext::drawTextureRegion(
        float x,
        float y,
        float width,
        float height,
        GLuint texture,
        float u,
        float v,
        float uw,
        float vh)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glEnable(GL_TEXTURE_2D);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glTexCoord2f(u, v);
        glVertex2f(x, y);

        glTexCoord2f(u, v + vh);
        glVertex2f(x, y + height);

        glTexCoord2f(u + uw, v + vh);
        glVertex2f(x + width, y + height);

        glTexCoord2f(u + uw, v);
        glVertex2f(x + width, y);

        glEnd();
    }

    void GraphicsContext::drawTextureRegion(
        float x,
        float y,
        float width,
        float height,
        const SharedTextureHandle& texture,
        float u,
        float v,
        float uw,
        float vh)
    {
        drawTextureRegion(x, y, width, height, texture.get(), u, v, uw, vh);
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, GLuint texture)
    {
        drawTextureRegion(x, y, width, height, texture, 0.0f, 0.0f, 1.0f, 1.0f);
    }

    void GraphicsContext::drawTexture(float x, float y, float width, float height, const SharedTextureHandle& texture)
    {
        drawTexture(x, y, width, height, texture.get());
    }

    void GraphicsContext::drawSprite(float x, float y, const Sprite& sprite)
    {
        drawTextureRegion(
            x + sprite.bounds.left(),
            y + sprite.bounds.top(),
            sprite.bounds.width(),
            sprite.bounds.height(),
            sprite.texture);
    }

    void GraphicsContext::applyCamera(const AbstractCamera& camera)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        multiplyMatrix(camera.getProjectionMatrix());

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        multiplyMatrix(camera.getViewMatrix());
    }

    void GraphicsContext::multiplyMatrix(const Matrix4f& m)
    {
        glMultMatrixf(m.data);
    }

    SharedTextureHandle
    GraphicsContext::createTexture(unsigned int width, unsigned int height, const std::vector<Color>& image)
    {
        assert(image.size() == width * height);

        unsigned int texture;
        glGenTextures(1, &texture);
        SharedTextureHandle handle(texture);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image.data());

        return handle;
    }

    SharedTextureHandle GraphicsContext::createColorTexture(Color c)
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        SharedTextureHandle handle(texture);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            1,
            1,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            &c);
        requireNoOpenGlError();

        return handle;
    }

    void GraphicsContext::drawText(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        auto it = utf8Begin(text);
        auto end = utf8End(text);
        for (; it != end; ++it)
        {
            auto ch = *it;
            if (ch > font.sprites.size())
            {
                ch = 0;
            }

            const auto& sprite = font.sprites[ch];

            if (ch != ' ')
            {
                drawSprite(x, y, sprite);
            }

            x += sprite.bounds.right();
        }
    }

    float GraphicsContext::getTextWidth(const std::string& text, const SpriteSeries& font)
    {
        float width = 0;
        auto it = utf8Begin(text);
        auto end = utf8End(text);
        for (; it != end; ++it)
        {
            auto ch = *it;
            if (ch > font.sprites.size())
            {
                ch = 0;
            }

            const auto& sprite = font.sprites[ch];

            width += sprite.bounds.right();
        }

        return width;
    }

    void GraphicsContext::drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        float width = getTextWidth(text, font);
        float halfWidth = width / 2.0f;
        float halfHeight = 5.0f;

        drawText(std::round(x - halfWidth), std::round(y + halfHeight), text, font);
    }

    void GraphicsContext::pushMatrix()
    {
        glPushMatrix();
    }

    void GraphicsContext::popMatrix()
    {
        glPopMatrix();
    }

    void GraphicsContext::fillColor(float x, float y, float width, float height, Color color)
    {
        glDisable(GL_TEXTURE_2D);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);

        glColor4ub(color.r, color.g, color.b, color.a);
        //glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

        glVertex2f(x, y);
        glVertex2f(x, y + height);
        glVertex2f(x + width, y + height);
        glVertex2f(x + width, y);

        glEnd();
    }

    void GraphicsContext::drawTextWrapped(Rectangle2f area, const std::string& text, const SpriteSeries& font)
    {
        float x = 0.0f;
        float y = 0.0f;

        auto it = utf8Begin(text);
        auto end = utf8End(text);
        while (it != end)
        {

            auto endOfWord = findEndOfWord(it, end);
            auto width = getTextWidth(it, endOfWord, font);

            if (x + width > area.width())
            {
                y += 15.0f;
                x = 0;
            }

            for (; it != endOfWord; ++it)
            {
                auto ch = *it;
                if (ch > font.sprites.size())
                {
                    ch = 0;
                }

                const auto& sprite = font.sprites[ch];

                drawSprite(area.left() + x, area.top() + y, sprite);

                x += sprite.bounds.right();
            }

            if (endOfWord != end)
            {
                const auto& spaceSprite = font.sprites[' '];
                x += spaceSprite.bounds.right();
                ++it;
            }
        }
    }

    void GraphicsContext::drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        glEnable(GL_TEXTURE_2D);

        // disable mipmapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                auto tilePosition = terrain.tileCoordinateToWorldCorner(x + dx, y + dy);

                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                glBindTexture(GL_TEXTURE_2D, tileTexture.texture.get());

                glBegin(GL_QUADS);

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glTexCoord2f(tileTexture.region.left(), tileTexture.region.top());
                glVertex3f(tilePosition.x, 0.0f, tilePosition.z);

                glTexCoord2f(tileTexture.region.left(), tileTexture.region.bottom());
                glVertex3f(tilePosition.x, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits);

                glTexCoord2f(tileTexture.region.right(), tileTexture.region.bottom());
                glVertex3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits);

                glTexCoord2f(tileTexture.region.right(), tileTexture.region.top());
                glVertex3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z);

                glEnd();
            }
        }
    }
}
