#include "ShaderService.h"
#include "UiRenderService.h"
#include "rwe_string.h"

namespace rwe
{
    UiRenderService::UiRenderService(GraphicsContext* graphics, ShaderService* shaders, const UiCamera& camera)
        : graphics(graphics), shaders(shaders), camera(camera)
    {
    }

    const UiCamera& UiRenderService::getCamera() const
    {
        return camera;
    }

    void UiRenderService::fillScreen(const Color& color)
    {
        fillColor(0.0f, 0.0f, camera.getWidth(), camera.getHeight(), color);
    }

    void UiRenderService::drawSprite(float x, float y, const Sprite& sprite)
    {
        auto matrix = matrixStack.top() * Matrix4f::translation(Vector3f(x, y, 0.0f));

        const auto& shader = shaders->basicTexture;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * matrix);
        graphics->setUniformFloat(shader.alpha, 1.0f);
        graphics->bindTexture(sprite.mesh.texture.get());

        graphics->drawTriangles(sprite.mesh.mesh);
    }

    void UiRenderService::drawSpriteAbs(float x, float y, const Sprite& sprite)
    {
        drawSprite(x - sprite.bounds.left(), y - sprite.bounds.top(), sprite);
    }

    void UiRenderService::drawText(float x, float y, const std::string& text, const SpriteSeries& font)
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

            const auto& sprite = *font.sprites[ch];

            if (ch != ' ')
            {
                drawSprite(x, y, sprite);
            }

            x += sprite.bounds.right();
        }
    }

    float UiRenderService::getTextWidth(const std::string& text, const SpriteSeries& font)
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

            const auto& sprite = *font.sprites[ch];

            width += sprite.bounds.right();
        }

        return width;
    }

    void UiRenderService::drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        float width = getTextWidth(text, font);
        float halfWidth = width / 2.0f;
        float halfHeight = 5.0f;

        drawText(std::round(x - halfWidth), std::round(y + halfHeight), text, font);
    }

    void UiRenderService::drawTextWrapped(Rectangle2f area, const std::string& text, const SpriteSeries& font)
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

                const auto& sprite = *font.sprites[ch];

                drawSprite(area.left() + x, area.top() + y, sprite);

                x += sprite.bounds.right();
            }

            if (endOfWord != end)
            {
                const auto& spaceSprite = *font.sprites[' '];
                x += spaceSprite.bounds.right();
                ++it;
            }
        }
    }

    void UiRenderService::fillColor(float x, float y, float width, float height, Color color)
    {

        auto floatColor = Vector3f(color.r, color.g, color.b) / 255.0f;
        std::vector<GlColoredVertex> vertices{
            {{x, y, 0.0f}, floatColor},
            {{x, y + height, 0.0f}, floatColor},
            {{x + width, y + height, 0.0f}, floatColor},

            {{x + width, y + height, 0.0f}, floatColor},
            {{x + width, y, 0.0f}, floatColor},
            {{x, y, 0.0f}, floatColor},
        };

        auto mesh = graphics->createColoredMesh(vertices, GL_STREAM_DRAW);

        glDisable(GL_TEXTURE_2D);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * matrixStack.top());
        graphics->drawTriangles(mesh);
    }

    void UiRenderService::pushMatrix()
    {
        matrixStack.push(matrixStack.top());
    }

    void UiRenderService::popMatrix()
    {
        matrixStack.pop();
    }

    void UiRenderService::multiplyMatrix(const Matrix4f& matrix)
    {
        auto replacement = matrixStack.top() * matrix;
        matrixStack.top() = replacement;
    }
}
