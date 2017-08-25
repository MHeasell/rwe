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
        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<SpriteSeries> _spriteSeries, std::string _label);

        void render(GraphicsContext& graphics) const override;

        bool mouseDown(MouseButtonEvent /*event*/) override;

        bool mouseUp(MouseButtonEvent /*event*/) override;

        void mouseEnter() override;

        void mouseLeave() override;

        void unfocus() override;
    };
}

#endif
