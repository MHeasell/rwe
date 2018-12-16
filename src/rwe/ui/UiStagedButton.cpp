#include "UiStagedButton.h"

namespace rwe
{
    void UiStagedButton::render(UiRenderService& graphics) const
    {
        auto spriteCount = spriteSeries->sprites.size();

        assert(spriteCount >= 3);
        auto pressedSprite = spriteCount - 2;

        assert(currentStage < spriteCount);
        auto stageSprite = currentStage;

        auto spriteIndex = pressed ? pressedSprite : stageSprite;
        const auto& sprite = *spriteSeries->sprites[spriteIndex];

        graphics.drawSpriteAbs(posX, posY, sizeX, sizeY, sprite);

        float textX = posX + 6.0f;
        float textY = posY + (sizeY / 2.0f) + 6.0f;
        graphics.drawText(textX, pressed ? textY + 1.0f : textY, labels[currentStage], *labelFont);
    }

    UiStagedButton::UiStagedButton(
        int posX,
        int posY,
        unsigned int sizeX,
        unsigned int sizeY,
        std::shared_ptr<SpriteSeries> _spriteSeries,
        std::vector<std::string> _labels,
        std::shared_ptr<SpriteSeries> _labelFont)
        : UiComponent(posX, posY, sizeX, sizeY),
          spriteSeries(std::move(_spriteSeries)),
          labels(std::move(_labels)),
          labelFont(std::move(_labelFont))
    {
        if (labels.size() != spriteSeries->sprites.size() - 3)
        {
            throw std::logic_error("Number of labels does not match number of sprites");
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
            auto stageCount = spriteSeries->sprites.size() - 3;
            currentStage = (currentStage + 1) % stageCount;
        }

        clickSubject.next(event);
        messagesSubject.next(ActivateMessage());
    }

    void UiStagedButton::setStage(unsigned int newStage)
    {
        if (newStage >= spriteSeries->sprites.size() - 3)
        {
            throw std::logic_error("New stage is not in range");
        }
        currentStage = newStage;
    }
}
