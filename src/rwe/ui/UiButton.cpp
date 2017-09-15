#include "UiButton.h"

namespace rwe
{
    UiButton::UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY,
        std::shared_ptr<SpriteSeries> _spriteSeries, std::string _label, std::shared_ptr<SpriteSeries> labelFont)
        : UiComponent(posX, posY, sizeX, sizeY), spriteSeries(std::move(_spriteSeries)), label(std::move(_label)), labelFont(std::move(labelFont))
    {
        assert(spriteSeries->sprites.size() >= 2);
    }

    void UiButton::render(GraphicsContext& graphics) const
    {
        const Sprite& sprite = pressed ? spriteSeries->sprites[1] : spriteSeries->sprites[0];
        graphics.drawTextureRegion(
            posX,
            posY,
            sizeX,
            sizeY,
            sprite.texture,
            sprite.textureRegion.left(),
            sprite.textureRegion.top(),
            sprite.textureRegion.width(),
            sprite.textureRegion.height());
        float textX = posX + (sizeX / 2.0f);
        float textY = posY + (sizeY / 2.0f);
        graphics.drawTextCentered(textX, pressed ? textY + 1.0f : textY, label, *labelFont);
    }

    void UiButton::mouseDown(MouseButtonEvent /*event*/)
    {
        armed = true;
        pressed = true;
    }

    void UiButton::mouseUp(MouseButtonEvent /*event*/)
    {
        if (armed && pressed)
        {
            clickSubject.next(true);
        }

        armed = false;
        pressed = false;
    }

    void UiButton::mouseEnter()
    {
        if (armed)
        {
            pressed = true;
        }
    }

    void UiButton::mouseLeave()
    {
        pressed = false;
    }

    void UiButton::unfocus()
    {
        armed = false;
        pressed = false;
    }

    Observable<bool>& UiButton::onClick()
    {
        return clickSubject;
    }

    void UiButton::keyDown(KeyEvent event)
    {
        if (event.keyCode == SDLK_RETURN || event.keyCode == SDLK_SPACE)
        {
            clickSubject.next(true);
        }
    }

    const std::string& UiButton::getLabel() const
    {
        return label;
    }

    void UiButton::setLabel(const std::string& newLabel)
    {
        label = newLabel;
    }

    void UiButton::setLabel(std::string&& newLabel)
    {
        label = std::move(newLabel);
    }
}
