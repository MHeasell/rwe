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

        auto info = getScrollBoxInfo();

        drawScrollBox(context, posX + 3.0f, posY + std::round(info.pos), std::round(info.size));
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

        const Sprite& upArrow = sprites->sprites[upArrowPressed ? 7 : 6];
        const Sprite& downArrow = sprites->sprites[downArrowPressed ? 9 : 8];

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

        drawSpriteAt(context, x, y, topBox);

        float bottomBoxStart = height - bottomBox.bounds.height();

        float yOffset = topBox.bounds.height();
        while (yOffset + middleBox.bounds.height() <= bottomBoxStart)
        {
            drawSpriteAt(context, x, y + yOffset, middleBox);
            yOffset += middleBox.bounds.height();
        }

        drawSpriteAt(context, x, y + yOffset, bottomBox);
    }

    void UiScrollBar::mouseDown(MouseButtonEvent event)
    {
        const Sprite& upArrow = sprites->sprites[6];
        if (Rectangle2f::fromTopLeft(posX, posY, upArrow.bounds.width(), upArrow.bounds.height()).contains(event.x, event.y))
        {
            upArrowPressed = true;
            return;
        }

        const Sprite& downArrow = sprites->sprites[8];
        if (Rectangle2f::fromTopLeft(posX, posY + sizeY - downArrow.bounds.height(), downArrow.bounds.width(), downArrow.bounds.height()).contains(event.x, event.y))
        {
            downArrowPressed = true;
            return;
        }

        auto boxInfo = getScrollBoxInfo();
        if (Rectangle2f::fromTopLeft(posX, posY + boxInfo.pos, sizeX, boxInfo.size).contains(event.x, event.y))
        {
            barGrabbed = true;
            mouseDownY = event.y;
            mouseDownScrollPercent = scrollPercent;
            return;
        }

        backgroundPressed = true;
        mouseY = event.y;
    }

    void UiScrollBar::mouseUp(MouseButtonEvent event)
    {
        upArrowPressed = false;
        downArrowPressed = false;
        barGrabbed = false;
        backgroundPressed = false;
    }

    void UiScrollBar::mouseMove(MouseMoveEvent event)
    {
        UiComponent::mouseMove(event);

        if (barGrabbed)
        {
            auto deltaPixels = static_cast<float>(event.y - mouseDownY);

            auto boxInfo = getScrollBoxInfo();

            float deltaPercent = deltaPixels / boxInfo.range;
            scrollPercent = std::clamp(mouseDownScrollPercent + deltaPercent, 0.0f, 1.0f);
        }

        if (backgroundPressed)
        {
            mouseY = event.y;
        }
    }

    UiScrollBar::ScrollBoxInfo UiScrollBar::getScrollBoxInfo() const
    {
        const Sprite& upArrow = sprites->sprites[6];
        const Sprite& downArrow = sprites->sprites[8];

        float topMargin = upArrow.bounds.height() + 3.0f;
        float bottomMargin = downArrow.bounds.height() + 3.0f;

        float boxRange = sizeY - topMargin - bottomMargin;
        float boxSize = scrollBarPercent * boxRange;
        float boxTopRange = boxRange - boxSize;

        float boxY = topMargin + (boxTopRange * scrollPercent);

        return ScrollBoxInfo{boxY, boxSize, boxTopRange};
    }

    float UiScrollBar::toScrollPercent(float pos)
    {
        const Sprite& upArrow = sprites->sprites[6];
        const Sprite& downArrow = sprites->sprites[8];

        float topMargin = upArrow.bounds.height() + 3.0f;
        float bottomMargin = downArrow.bounds.height() + 3.0f;

        float boxRange = sizeY - topMargin - bottomMargin;

        return (pos - topMargin) / boxRange;
    }

    void UiScrollBar::update(float dt)
    {
        // speed in percent/sec
        float scrollSpeed = 4.0f;

        if (backgroundPressed)
        {
            auto cursorPercent = toScrollPercent(mouseY - posY);

            auto boxTopPercent = scrollPercent * (1.0f - scrollBarPercent);

            if (cursorPercent < boxTopPercent) // cursor above
            {
                scrollPercent = std::clamp(scrollPercent - (scrollSpeed * dt), 0.0f, 1.0f);
            }
            else if (cursorPercent > boxTopPercent + scrollBarPercent) // cursor below
            {
                scrollPercent = std::clamp(scrollPercent + (scrollSpeed * dt), 0.0f, 1.0f);
            }

            return;
        }
    }

    void UiScrollBar::uiMessage(const GroupMessage& message)
    {
        if (message.group != group)
        {
            return;
        }

        auto scrollMessage = boost::get<ScrollPositionMessage>(&(message.message));
        if (!scrollMessage)
        {
            return;
        }

        scrollPercent = scrollMessage->scrollPosition;
        scrollBarPercent = scrollMessage->scrollViewportPercent;
    }
}
