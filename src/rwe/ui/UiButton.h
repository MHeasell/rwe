#ifndef RWE_UIBUTTON_H
#define RWE_UIBUTTON_H

#include <rwe/ui/UiComponent.h>
#include <vector>
#include <functional>

namespace rwe
{
    class UiButton : public UiComponent
    {
    private:
        GLuint normalTexture;
        GLuint pressedTexture;

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
        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, GLuint normalTexture, GLuint pressedTexture)
            : UiComponent(posX, posY, sizeX, sizeY), normalTexture(normalTexture), pressedTexture(pressedTexture)
        {
        }

        void render(GraphicsContext& graphics) const override
        {
            GLuint texture = pressed ? pressedTexture : normalTexture;
            graphics.drawTexture(posX, posY, sizeX, sizeY, texture);
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
