#include "UiScrollBar.h"

namespace rwe
{
    void drawSpriteAt(GraphicsContext& graphics, float x, float y, const Sprite& sprite)
    {
        graphics.drawTextureRegion(
            x,
            y,
            sprite.bounds.width(),
            sprite.bounds.height(),
            sprite.texture,
            sprite.textureRegion.left(),
            sprite.textureRegion.top(),
            sprite.textureRegion.width(),
            sprite.textureRegion.height());
    }

    void UiScrollBar::render(GraphicsContext& context) const
    {
        drawScrollBackground(context, posX, posY, sizeY);

        const Sprite& upArrow = sprites->sprites[6];
        const Sprite& downArrow = sprites->sprites[8];

        float topMargin = upArrow.bounds.height() + 3.0f;
        float bottomMargin = downArrow.bounds.height() + 3.0f;

        float boxRange = sizeY - topMargin - bottomMargin;
        float boxSize = scrollBarPercent * boxRange;
        float boxTopRange = boxRange - boxSize;
        float boxY = topMargin + (boxTopRange * scrollPercent);

        drawScrollBox(context, posX + 3.0f, posY + boxY, boxSize);
    }

    UiScrollBar::UiScrollBar(
        int posX,
        int posY,
        unsigned int sizeX,
        unsigned int sizeY,
        std::shared_ptr<SpriteSeries> sprites)
        : UiComponent(posX, posY, sizeX, sizeY),
          sprites(std::move(sprites))
    {
    }

    void UiScrollBar::drawScrollBackground(GraphicsContext& graphics, float x, float y, float height) const
    {
        const Sprite& topBackground = sprites->sprites[0];
        const Sprite& middleBackground = sprites->sprites[1];
        const Sprite& bottomBackground = sprites->sprites[2];

        const Sprite& upArrow = sprites->sprites[6];
        const Sprite& downArrow = sprites->sprites[8];

        float bottomMargin = bottomBackground.bounds.height() + downArrow.bounds.height();

        float yOffset = 0.0f;

        drawSpriteAt(graphics, x, y + yOffset, upArrow);
        yOffset += upArrow.bounds.height();

        drawSpriteAt(graphics, x, y + yOffset, topBackground);
        yOffset += topBackground.bounds.height();


        while (yOffset < height - bottomMargin)
        {
            drawSpriteAt(graphics, x, y + yOffset, middleBackground);
            yOffset += middleBackground.bounds.height();
        }

        drawSpriteAt(graphics, x, y + height - bottomMargin, bottomBackground);
        bottomMargin -= bottomBackground.bounds.height();

        drawSpriteAt(graphics, x, y + height - bottomMargin, downArrow);
    }

    void UiScrollBar::drawScrollBox(GraphicsContext& context, float x, float y, float height) const
    {
        const Sprite& topBox = sprites->sprites[3];
        const Sprite& middleBox = sprites->sprites[4];
        const Sprite& bottomBox = sprites->sprites[5];

        float bottomMargin = bottomBox.bounds.height();

        drawSpriteAt(context, x, y, topBox);

        float yOffset = topBox.bounds.height();
        while (yOffset < height - bottomMargin)
        {
            drawSpriteAt(context, x, y + yOffset, middleBox);
            yOffset += middleBox.bounds.height();
        }

        drawSpriteAt(context, x, y + height - bottomMargin, bottomBox);
    }
}
