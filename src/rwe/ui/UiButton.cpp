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
            sprite.bounds.width(),
            sprite.bounds.height(),
            sprite.texture,
            sprite.textureRegion.left(),
            sprite.textureRegion.top(),
            sprite.textureRegion.width(),
            sprite.textureRegion.height()
        );
        graphics.drawText(posX, posY, label, *labelFont);
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
            for (const auto& e : clickObservers)
            {
                e(event);
            }
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

    void UiButton::onClick(const std::function<void(MouseButtonEvent)>& callback)
    {
        clickObservers.push_back(callback);
    }
}
