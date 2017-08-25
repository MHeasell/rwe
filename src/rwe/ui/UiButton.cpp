#include "UiButton.h"

namespace rwe
{
    UiButton::UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY,
                       std::shared_ptr<SpriteSeries> _spriteSeries, std::string _label)
        : UiComponent(posX, posY, sizeX, sizeY), spriteSeries(std::move(_spriteSeries)), label(std::move(_label))
    {
        assert(spriteSeries->sprites.size() >= 2);
    }

    void UiButton::render(GraphicsContext& graphics) const
    {
        const Sprite& sprite = pressed ? spriteSeries->sprites[1] : spriteSeries->sprites[0];
        graphics.drawTexture(posX, posY, sprite.bounds.width(), sprite.bounds.height(), sprite.texture);
    }

    bool UiButton::mouseDown(MouseButtonEvent)
    {
        armed = true;
        pressed = true;
        return true;
    }

    bool UiButton::mouseUp(MouseButtonEvent)
    {
        if (armed && pressed)
        {
            // TODO: call an event handler or something here
        }

        armed = false;
        pressed = false;

        return true;
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
}
