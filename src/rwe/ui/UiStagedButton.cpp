#include "UiStagedButton.h"

namespace rwe
{
    ButtonClickEvent::Source mouseButtonToSource(MouseButtonEvent::MouseButton btn)
    {
        switch (btn)
        {
            case MouseButtonEvent::MouseButton::Left:
                return ButtonClickEvent::Source::LeftMouseButton;
            case MouseButtonEvent::MouseButton::Right:
                return ButtonClickEvent::Source::RightMouseButton;
            case MouseButtonEvent::MouseButton::Middle:
                return ButtonClickEvent::Source::MiddleMouseButton;
                break;
            default:
                throw std::logic_error("Unknown mouse button value");
        }
    }

    void UiStagedButton::render(UiRenderService& graphics) const
    {
        const auto& sprite = pressed ? *pressedSprite : *stages[currentStage].sprite;

        graphics.drawSpriteAbs(posX, posY, sizeX, sizeY, sprite);

        const auto& label = stages[currentStage].label;
        switch (textAlign)
        {
            case TextAlign::Left:
            {
                float textX = posX + 6.0f;
                float textY = posY + (sizeY / 2.0f) + 6.0f;
                if (pressed)
                {
                    textX += 1.0f;
                    textY += 1.0f;
                }
                graphics.drawText(textX, textY, label, *labelFont);
                break;
            }
            case TextAlign::Center:
            {
                float textX = posX + (sizeX / 2.0f);
                float textY = posY + (sizeY / 2.0f);
                if (pressed)
                {
                    textX += 1.0f;
                    textY += 1.0f;
                }
                graphics.drawTextCentered(textX, textY, label, *labelFont);
                break;
            }
            default:
                throw std::logic_error("Invalid TextAlign value");
        }
    }

    UiStagedButton::UiStagedButton(
        int posX,
        int posY,
        unsigned int sizeX,
        unsigned int sizeY,
        std::vector<StageInfo> stages,
        std::shared_ptr<Sprite> pressedSprite,
        std::shared_ptr<SpriteSeries> labelFont)
        : UiComponent(posX, posY, sizeX, sizeY),
          stages(std::move(stages)),
          pressedSprite(std::move(pressedSprite)),
          labelFont(std::move(labelFont))
    {
        if (this->stages.empty())
        {
            throw std::logic_error("No stages provided for staged button");
        }
    }

    void UiStagedButton::mouseDown(MouseButtonEvent event)
    {
        switch (activateOn)
        {
            case ActivateMode::MouseDown:
                activateButton({(mouseButtonToSource(event.button))});
                break;
            case ActivateMode::MouseUp:
                armed = true;
                pressed = true;
                break;
            default:
                throw std::logic_error("Invalid ActivateMode");
        }
    }

    void UiStagedButton::mouseUp(MouseButtonEvent event)
    {
        switch (activateOn)
        {
            case ActivateMode::MouseDown:
                break;
            case ActivateMode::MouseUp:
                if (armed && pressed)
                {
                    activateButton({(mouseButtonToSource(event.button))});
                }

                armed = false;
                pressed = false;
                break;
            default:
                throw std::logic_error("Invalid ActivateMode");
        }
    }

    void UiStagedButton::mouseEnter()
    {
        if (activateOn == ActivateMode::MouseUp)
        {
            if (armed)
            {
                pressed = true;
            }
        }
    }

    void UiStagedButton::mouseLeave()
    {
        if (activateOn == ActivateMode::MouseUp)
        {
            pressed = false;
        }
    }

    void UiStagedButton::unfocus()
    {
        if (activateOn == ActivateMode::MouseUp)
        {
            armed = false;
            pressed = false;
        }
    }

    Observable<ButtonClickEvent>& UiStagedButton::onClick()
    {
        return clickSubject;
    }

    void UiStagedButton::keyDown(KeyEvent event)
    {
        if (event.keyCode == SDLK_SPACE)
        {
            activateButton({ButtonClickEvent::Source::Keyboard});
        }
    }

    void UiStagedButton::activateButton(const ButtonClickEvent& event)
    {
        if (autoChangeStage)
        {
            auto stageCount = stages.size();
            currentStage = (currentStage + 1) % stageCount;
        }

        clickSubject.next(event);
        messagesSubject.next(ActivateMessage{sourceToType(event.source)});
    }

    void UiStagedButton::setStage(unsigned int newStage)
    {
        if (newStage >= stages.size())
        {
            throw std::logic_error("New stage is not in range");
        }
        currentStage = newStage;
    }

    void UiStagedButton::setTextAlign(UiStagedButton::TextAlign align)
    {
        textAlign = align;
    }

    void UiStagedButton::setLabel(const std::string& label)
    {
        stages[currentStage].label = label;
    }

    void UiStagedButton::setNormalSprite(const std::shared_ptr<Sprite>& sprite)
    {
        stages[currentStage].sprite = sprite;
    }

    void UiStagedButton::setPressedSprite(const std::shared_ptr<Sprite>& sprite)
    {
        pressedSprite = sprite;
    }

    void UiStagedButton::setPressed(bool _pressed)
    {
        pressed = _pressed;
    }

    void UiStagedButton::setActivateMode(rwe::UiStagedButton::ActivateMode mode)
    {
        activateOn = mode;
    }
}
