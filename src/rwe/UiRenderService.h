#ifndef RWE_UIRENDERSERVICE_H
#define RWE_UIRENDERSERVICE_H

#include <rwe/GraphicsContext.h>
#include <rwe/ShaderService.h>
#include <rwe/camera/UiCamera.h>
#include <stack>

namespace rwe
{
    class UiRenderService
    {
    private:
        GraphicsContext* graphics;
        ShaderService* shaders;
        UiCamera camera;

        std::stack<Matrix4f> matrixStack{{Matrix4f::identity()}};

    public:
        UiRenderService(GraphicsContext* graphics, ShaderService* shaders, const UiCamera& camera);

        const UiCamera& getCamera() const;

        void fillScreen(const Color& color);

        void drawSprite(float x, float y, const Sprite& sprite);

        /** Draws the sprite, ignoring its internal x and y offset. */
        void drawSpriteAbs(float x, float y, const Sprite& sprite);

        void drawSpriteAbs(float x, float y, float width, float height, const Sprite& sprite);

        void drawText(float x, float y, const std::string& text, const SpriteSeries& font);

        void drawTextWrapped(Rectangle2f area, const std::string& text, const SpriteSeries& font);

        void drawTextCentered(float x, float y, const std::string& text, const SpriteSeries& font);

        float getTextWidth(const std::string& text, const SpriteSeries& font);

        void pushMatrix();

        void popMatrix();

        /**
         * Multiplies the current OpenGL matrix with the specified matrix.
         * If the current OpenGL matrix is C and the coordinates to be transformed are v,
         * meaning that the current transformation would be C * v,
         * then calling this with an argument M replaces the current transformation with (C * M) * v.
         * Intuitively this means that any transformation you pass in here
         * will be done directly on the coordinates,
         * and the existing tranformation will be done on the result of that.
         */
        void multiplyMatrix(const Matrix4f& matrix);

        template <typename It>
        float getTextWidth(It begin, It end, const SpriteSeries& font);

        template <typename It>
        It findEndOfWord(It it, It end);

        void fillColor(float x, float y, float width, float height, Color color);

        void drawHealthBar(float x, float y, float percentFull);

        void drawBoxOutline(float x, float y, float width, float height, Color color);
    };

    template <typename It>
    float UiRenderService::getTextWidth(It it, It end, const SpriteSeries& font)
    {
        float width = 0;
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

    template <typename It>
    It UiRenderService::findEndOfWord(It it, It end)
    {
        return std::find_if(it, end, [](int ch) { return ch == ' '; });
    }
}

#endif
