#include "UiButton.h"

namespace rwe
{
    UiButton::UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> _spriteSeries, std::string _label, std::shared_ptr<SpriteSeries> labelFont)
        : UiComponent(posX, posY, sizeX, sizeY), spriteSeries(std::move(_spriteSeries)), label(std::move(_label)), labelFont(std::move(labelFont))
    {
        assert(spriteSeries->sprites.size() >= 2);
    }

    void UiButton::render(UiRenderService& graphics) const
    {
        const Sprite& sprite = pressed ? spriteSeries->sprites[1] : spriteSeries->sprites[0];
        graphics.drawTextureRegion(
            posX,
            posY,
            sizeX,
            sizeY,
            sprite.texture);
        float textX = posX + (sizeX / 2.0f);
        float textY = posY + (sizeY / 2.0f);
        graphics.drawTextCentered(textX, pressed ? textY + 1.0f : textY, label, *labelFont);
    }

    void UiButton::mouseDown(MouseButtonEvent /*event*/)
    {
        armed = true;
        pressed = true;
    }

    void UiButton::mouseUp(MouseButtonEvent event)
    {
        if (armed && pressed)
        {
            ButtonClickEvent::Source s;
            switch (event.button)
            {
                case MouseButtonEvent::MouseButton::Left:
                    s = ButtonClickEvent::Source::LeftMouseButton;
                    break;
                case MouseButtonEvent::MouseButton::Right:
                    s = ButtonClickEvent::Source::RightMouseButton;
                    break;
                case MouseButtonEvent::MouseButton::Middle:
                    s = ButtonClickEvent::Source::MiddleMouseButton;
                    break;
            }
            clickSubject.next({s});
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

    Observable<ButtonClickEvent>& UiButton::onClick()
    {
        return clickSubject;
    }

    void UiButton::keyDown(KeyEvent event)
    {
        if (event.keyCode == SDLK_RETURN || event.keyCode == SDLK_SPACE)
        {
            clickSubject.next({ButtonClickEvent::Source::Keyboard});
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
