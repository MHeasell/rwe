#include "UiStagedButton.h"

namespace rwe
{
    void UiStagedButton::render(UiRenderService& graphics) const
    {
        const auto& sprite = pressed ? *pressedSprite : *stages[currentStage].sprite;

        graphics.drawSpriteAbs(posX, posY, sizeX, sizeY, sprite);

        float textX = posX + 6.0f;
        float textY = posY + (sizeY / 2.0f) + 6.0f;
        if (pressed)
        {
            textY += 1.0f;
        }
        const auto& label = stages[currentStage].label;
        switch(textAlign)
        {
            case TextAlign::Left:
                graphics.drawText(textX, textY, label, *labelFont);
                break;
            case TextAlign::Center:
                graphics.drawTextCentered(textX, textY, label, *labelFont);
                break;
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

    void UiStagedButton::mouseDown(MouseButtonEvent)
    {
        armed = true;
        pressed = true;
    }

    void UiStagedButton::mouseUp(MouseButtonEvent event)
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

    void UiStagedButton::mouseEnter()
    {
        if (armed)
        {
            pressed = true;
        }
    }

    void UiStagedButton::mouseLeave()
    {
        pressed = false;
    }

    void UiStagedButton::unfocus()
    {
        armed = false;
        pressed = false;
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
}
