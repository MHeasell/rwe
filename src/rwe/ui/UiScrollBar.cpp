#include "UiScrollBar.h"

namespace rwe
{
    void drawSpriteAt(float x, float y, const Sprite& sprite, GraphicsContext& graphics)
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
        const Sprite& topBackground = sprites->sprites[0];
        const Sprite& middleBackground = sprites->sprites[1];
        const Sprite& bottomBackground = sprites->sprites[2];

        const Sprite& upArrow = sprites->sprites[6];
        const Sprite& downArrow = sprites->sprites[8];

        float y = 0.0f;

        drawSpriteAt(posX, posY + y, upArrow, context);
        y += upArrow.bounds.height();

        drawSpriteAt(posX, posY + y, topBackground, context);
        y += topBackground.bounds.height();

        float bottomMargin = bottomBackground.bounds.height() + downArrow.bounds.height();

        while (y < sizeY - bottomMargin)
        {
            drawSpriteAt(posX, posY + y, middleBackground, context);
            y += middleBackground.bounds.height();
        }

        drawSpriteAt(posX, posY + sizeY - bottomMargin, bottomBackground, context);
        bottomMargin -= bottomBackground.bounds.height();

        drawSpriteAt(posX, posY + sizeY - bottomMargin, downArrow, context);
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
}
