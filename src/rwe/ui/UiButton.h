#ifndef RWE_UIBUTTON_H
#define RWE_UIBUTTON_H

#include <functional>
#include <rwe/SpriteSeries.h>
#include <rwe/ui/UiComponent.h>
#include <vector>

namespace rwe
{
    class UiButton : public UiComponent
    {
    private:
        std::shared_ptr<SpriteSeries> spriteSeries;
        std::string label;

        /** True if the button is currently pressed down. */
        bool pressed{false};

        /**
         * True if the button is "armed".
         * The button is armed if the mouse cursor was pressed down inside of it
         * and has not yet been released.
         */
        bool armed{false};

        std::vector<std::function<bool(MouseButtonEvent)>> clickObservers;

    public:
        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> _spriteSeries, std::string _label)
            : UiComponent(posX, posY, sizeX, sizeY), spriteSeries(std::move(_spriteSeries)), label(std::move(_label))
        {
            assert(spriteSeries->sprites.size() >= 2);
        }

        void render(GraphicsContext& graphics) const override
        {
            const Sprite& sprite = pressed ? spriteSeries->sprites[1] : spriteSeries->sprites[0];
            graphics.drawTexture(posX, posY, sprite.bounds.width(), sprite.bounds.height(), sprite.texture);
        }

        bool mouseDown(MouseButtonEvent /*event*/) override
        {
            armed = true;
            pressed = true;
            return true;
        }

        bool mouseUp(MouseButtonEvent /*event*/) override
        {
            if (armed && pressed)
            {
                // TODO: call an event handler or something here
            }

            armed = false;
            pressed = false;

            return true;
        }

        void mouseEnter() override
        {
            if (armed)
            {
                pressed = true;
            }
        }

        void mouseLeave() override
        {
            pressed = false;
        }

        void unfocus() override
        {
            armed = false;
            pressed = false;
        }
    };
}

#endif
