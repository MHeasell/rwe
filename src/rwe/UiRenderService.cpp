#include "UiRenderService.h"
#include <rwe/ShaderService.h>
#include <rwe/util/rwe_string.h>

namespace rwe
{
    UiRenderService::UiRenderService(GraphicsContext* graphics, ShaderService* shaders, const AbstractViewport* viewport)
        : graphics(graphics), shaders(shaders), viewport(viewport)
    {
    }

    void UiRenderService::fillScreen(const Color& color)
    {
        fillColor(0.0f, 0.0f, viewport->width(), viewport->height(), color);
    }

    void UiRenderService::drawSprite(float x, float y, const Sprite& sprite)
    {
        drawSprite(x, y, sprite, Color(255, 255, 255));
    }

    void UiRenderService::drawSprite(float x, float y, const Sprite& sprite, const Color& tint)
    {
        auto matrix = matrixStack.top()
            * Matrix4f::translation(Vector3f(x, y, 0.0f))
            * sprite.getTransform();

        const auto& shader = shaders->basicTexture;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, getViewProjectionMatrix() * matrix);
        graphics->setUniformVec4(shader.tint, tint.r / 255.0f, tint.g / 255.0f, tint.b / 255.0f, tint.a / 255.0f);
        graphics->bindTexture(sprite.texture.get());

        graphics->drawTriangles(*sprite.mesh);
    }

    void UiRenderService::drawSpriteAbs(float x, float y, const Sprite& sprite)
    {
        drawSprite(x - sprite.bounds.left(), y - sprite.bounds.top(), sprite);
    }

    void UiRenderService::drawSpriteAbs(float x, float y, float width, float height, const Sprite& sprite)
    {
        auto matrix = Matrix4f::translation(Vector3f(x, y, 0.0f))
            * Matrix4f::scale(Vector3f(width / sprite.bounds.width(), height / sprite.bounds.height(), 1.0f))
            * Matrix4f::translation(Vector3f(-sprite.bounds.left(), -sprite.bounds.top(), 0.0f));

        pushMatrix();
        multiplyMatrix(matrix);
        drawSprite(0.0f, 0.0f, sprite);
        popMatrix();
    }

    void UiRenderService::drawSpriteAbs(const Rectangle2f& rect, const Sprite& sprite)
    {
        drawSpriteAbs(rect.left(), rect.top(), rect.width(), rect.height(), sprite);
    }

    void UiRenderService::drawText(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        drawText(x, y, text, font, Color(255, 255, 255));
    }

    void UiRenderService::drawText(float x, float y, const std::string& text, const SpriteSeries& font, const Color& tint)
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
                drawSprite(x, y, sprite, tint);
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

    void UiRenderService::drawTextCenteredX(float x, float y, const std::string& text, const rwe::SpriteSeries& font)
    {
        float width = getTextWidth(text, font);
        float halfWidth = width / 2.0f;

        drawText(std::round(x - halfWidth), y, text, font);
    }

    void UiRenderService::drawTextAlignRight(float x, float y, const std::string& text, const SpriteSeries& font)
    {
        drawTextAlignRight(x, y, text, font, Color(255, 255, 255));
    }

    void UiRenderService::drawTextAlignRight(float x, float y, const std::string& text, const SpriteSeries& font, const Color& tint)
    {
        auto width = getTextWidth(text, font);
        drawText(x - width, y, text, font, tint);
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
        assert(width >= 0.0f);
        assert(height >= 0.0f);

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

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, getViewProjectionMatrix() * matrixStack.top());
        graphics->setUniformFloat(shader.alpha, static_cast<float>(color.a) / 255.0f);
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

    Color getHealthColor(float fractionFull)
    {
        if (fractionFull < 1.0f / 3.0f)
        {
            return Color(255, 71, 0);
        }

        if (fractionFull < 2.0f / 3.0f)
        {
            return Color(247, 227, 103);
        }

        return Color(83, 223, 79);
    }

    void UiRenderService::drawHealthBar(float x, float y, float percentFull)
    {
        x = std::floor(x);
        y = std::floor(y);

        // offset the healtbar from the unit centre
        x -= 17.0f;
        y += 8.0f;

        auto width = 35.0f;
        auto height = 5.0f;
        auto borderWidth = 1.0f;
        Color borderColor = Color::Black;

        auto innerMaxWidth = width - (borderWidth * 2.0f);
        auto innerWidth = 1.0f + std::floor(percentFull * (innerMaxWidth - 1.0f));
        auto innerHeight = height - (borderWidth * 2.0f);
        auto healthColor = getHealthColor(percentFull);

        fillColor(x, y, width, height, borderColor);
        fillColor(x + borderWidth, y + borderWidth, innerWidth, innerHeight, healthColor);
    }

    void UiRenderService::drawHealthBar2(float x, float y, float width, float height, float percentFull)
    {
        assert(percentFull >= 0.0f && percentFull <= 1.0f);
        float healthWidth = width * percentFull;
        fillColor(x, y, healthWidth, height, Color(83, 223, 79));
        fillColor(x + healthWidth, y, width - healthWidth, height, Color(171, 23, 0));
    }

    void UiRenderService::drawBoxOutline(float x, float y, float width, float height, Color color)
    {
        drawBoxOutline(x, y, width, height, color, 1.0f);
    }

    void UiRenderService::drawBoxOutline(float x, float y, float width, float height, Color color, float thickness)
    {
        assert(width >= 0.0f);
        assert(height >= 0.0f);

        if (width < 1.0f || height < 1.0f)
        {
            return;
        }

        if (width <= thickness * 2.0f || height <= thickness * 2.0f)
        {
            fillColor(x, y, width, height, color);
            return;
        }

        // top
        fillColor(x, y, width, thickness, color);
        // bottom
        fillColor(x, y + height - thickness, width, thickness, color);
        // left
        fillColor(x, y + thickness, thickness, height - (thickness * 2.0f), color);
        // right
        fillColor(x + width - thickness, y + thickness, thickness, height - (thickness * 2.0f), color);
    }

    void UiRenderService::drawLine(const Vector2f& start, const Vector2f& end)
    {
        auto floatColor = Vector3f(1.0f, 1.0f, 1.0f);
        std::vector<GlColoredVertex> vertices{
            {{start.x, start.y, 0.0f}, floatColor},
            {{end.x, end.y, 0.0f}, floatColor},
        };

        auto mesh = graphics->createColoredMesh(vertices, GL_STREAM_DRAW);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, getViewProjectionMatrix() * matrixStack.top());
        graphics->setUniformFloat(shader.alpha, 1.0f);
        graphics->drawLines(mesh);
    }

    Matrix4f UiRenderService::getViewProjectionMatrix() const
    {
        return Matrix4f::orthographicProjection(0.0f, viewport->width(), viewport->height(), 0.0f, 100.0f, -100.0f);
    }

    Matrix4f UiRenderService::getInverseViewProjectionMatrix() const
    {
        return Matrix4f::inverseOrthographicProjection(0.0f, viewport->width(), viewport->height(), 0.0f, 100.0f, -100.0f);
    }
}
