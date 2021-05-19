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
        const auto& sprite = pressed || toggledOn ? *pressedSprite : *stages[currentStage].sprite;

        graphics.drawSpriteAbs(static_cast<float>(posX), static_cast<float>(posY), sprite);

        const auto& label = stages[currentStage].label;
        switch (textAlign)
        {
            case TextAlign::Hidden:
                break;
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
            case TextAlign::BottomCenter:
            {
                float textX = posX + (sizeX / 2.0f);
                float textY = posY + sizeY - 7.f;
                if (pressed)
                {
                    textX += 1.0f;
                    textY += 1.0f;
                }
                graphics.drawTextCenteredX(textX, textY, label, *labelFont);
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
        switch (behaviorMode)
        {
            case BehaviorMode::Radio:
            case BehaviorMode::Cycle:
                activateButton({(mouseButtonToSource(event.button))});
                break;
            case BehaviorMode::Toggle:
            case BehaviorMode::Button:
            case BehaviorMode::Staged:
                armed = true;
                pressed = true;
                break;
            default:
                throw std::logic_error("Invalid BehaviorMode");
        }
    }

    void UiStagedButton::mouseUp(MouseButtonEvent event)
    {
        switch (behaviorMode)
        {
            case BehaviorMode::Radio:
            case BehaviorMode::Cycle:
                break;
            case BehaviorMode::Button:
            case BehaviorMode::Toggle:
            case BehaviorMode::Staged:
                armed = false;
                if (pressed)
                {
                    pressed = false;
                    activateButton({(mouseButtonToSource(event.button))});
                }
                break;
            default:
                throw std::logic_error("Invalid BehaviorMode");
        }
    }

    void UiStagedButton::mouseEnter()
    {
        switch (behaviorMode)
        {
            case BehaviorMode::Button:
            case BehaviorMode::Toggle:
            case BehaviorMode::Staged:
                if (armed)
                {
                    pressed = true;
                }
                break;
            default:
                break;
        }
    }

    void UiStagedButton::mouseLeave()
    {
        switch (behaviorMode)
        {
            case BehaviorMode::Button:
            case BehaviorMode::Toggle:
            case BehaviorMode::Staged:
                pressed = false;
                break;
            default:
                break;
        }
    }

    void UiStagedButton::unfocus()
    {
        switch (behaviorMode)
        {
            case BehaviorMode::Button:
            case BehaviorMode::Toggle:
            case BehaviorMode::Staged:
                armed = false;
                pressed = false;
                break;
            default:
                break;
        }
    }

    Observable<ButtonClickEvent>& UiStagedButton::onClick()
    {
        return clickSubject;
    }

    void UiStagedButton::keyDown(KeyEvent event)
    {
        if (event.keyCode == SDLK_SPACE || (quickKey && event.keyCode == quickKey))
        {
            activateButton({ButtonClickEvent::Source::Keyboard});
        }
    }

    void UiStagedButton::activateButton(const ButtonClickEvent& event)
    {
        switch (behaviorMode)
        {
            case BehaviorMode::Radio:
                toggledOn = true;
                break;
            case BehaviorMode::Cycle:
                nextStage();
                break;
            case BehaviorMode::Toggle:
                toggledOn = !toggledOn;
                break;
            case BehaviorMode::Button:
            case BehaviorMode::Staged:
                break;
            default:
                throw std::logic_error("Invalid BehaviorMode");
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

    void UiStagedButton::setToggledOn(bool _toggledOn)
    {
        toggledOn = _toggledOn;
    }

    void UiStagedButton::setBehaviorMode(BehaviorMode mode)
    {
        behaviorMode = mode;
    }

    void UiStagedButton::setQuickKey(int quickKey)
    {
        this->quickKey = quickKey;
    }

    void UiStagedButton::nextStage()
    {
        auto stageCount = stages.size();
        currentStage = (currentStage + 1) % stageCount;
    }
}
