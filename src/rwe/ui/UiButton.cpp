#include "UiButton.h"

namespace rwe
{
    UiButton::UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::shared_ptr<SpriteSeries>& sprites, std::string _label, std::shared_ptr<SpriteSeries> labelFont)
            : UiComponent(posX, posY, sizeX, sizeY), normalSprite(sprites->sprites.at(0)), pressedSprite(sprites->sprites.at(1)), label(std::move(_label)), labelFont(std::move(labelFont))
    {
    }

    UiButton::UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> _normalSprite, std::shared_ptr<Sprite> _pressedSprite, std::string _label, std::shared_ptr<SpriteSeries> labelFont)
        : UiComponent(posX, posY, sizeX, sizeY), normalSprite(std::move(_normalSprite)), pressedSprite(std::move(_pressedSprite)), label(std::move(_label)), labelFont(std::move(labelFont))
    {
    }

    void UiButton::render(UiRenderService& graphics) const
    {
        const auto& sprite = pressed ? *pressedSprite : *normalSprite;

        graphics.drawSpriteAbs(posX, posY, sizeX, sizeY, sprite);
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
            activateButton({s});
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
            activateButton({ButtonClickEvent::Source::Keyboard});
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

    void UiButton::setNormalSprite(const std::shared_ptr<Sprite>& sprite)
    {
        normalSprite = sprite;
    }

    void UiButton::setPressedSprite(const std::shared_ptr<Sprite>& sprite)
    {
        pressedSprite = sprite;
    }

    void UiButton::activateButton(const ButtonClickEvent& event)
    {
        clickSubject.next(event);
        messagesSubject.next(ActivateMessage{sourceToType(event.source)});
    }
}
