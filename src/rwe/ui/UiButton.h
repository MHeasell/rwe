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

        std::vector<std::function<bool(UiMouseButtonEvent)>> clickObservers;

    public:
        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> spriteSeries, std::string label)
            : UiComponent(posX, posY, sizeX, sizeY), spriteSeries(std::move(spriteSeries)), label(std::move(label))
        {
            assert(spriteSeries->sprites.size() >= 2);
        }

        void render(GraphicsContext& graphics) const override
        {
            const Sprite& sprite = pressed ? spriteSeries->sprites[0] : spriteSeries->sprites[1];
            graphics.drawSprite(posX, posY, sprite);
        }

        bool mouseDown(UiMouseButtonEvent /*event*/) override
        {
            armed = true;
            pressed = true;
            return true;
        }

        bool mouseUp(UiMouseButtonEvent /*event*/) override
        {
            if (armed && pressed)
            {
                // TODO: call an event handler or something here
                return true;
            }

            armed = false;
            pressed = false;

            return false;
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
