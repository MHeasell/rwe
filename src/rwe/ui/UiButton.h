#ifndef RWE_UIBUTTON_H
#define RWE_UIBUTTON_H

#include <functional>
#include <memory>
#include <rwe/SpriteSeries.h>
#include <rwe/observable/Subject.h>
#include <rwe/ui/UiComponent.h>
#include <vector>

namespace rwe
{
    class UiButton : public UiComponent
    {
    private:
        std::shared_ptr<Sprite> normalSprite;
        std::shared_ptr<Sprite> pressedSprite;
        std::string label;
        std::shared_ptr<SpriteSeries> labelFont;

        /** True if the button is currently pressed down. */
        bool pressed{false};

        /**
         * True if the button is "armed".
         * The button is armed if the mouse cursor was pressed down inside of it
         * and has not yet been released.
         */
        bool armed{false};

        Subject<ButtonClickEvent> clickSubject;

    public:
        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, const std::shared_ptr<SpriteSeries>& _sprites, std::string _label, std::shared_ptr<SpriteSeries> labelFont);

        UiButton(int posX, int posY, unsigned int sizeX, unsigned int sizeY, std::shared_ptr<Sprite> _normalSprite, std::shared_ptr<Sprite> _pressedSprite, std::string _label, std::shared_ptr<SpriteSeries> labelFont);

        void render(UiRenderService& graphics) const override;

        void mouseDown(MouseButtonEvent /*event*/) override;

        void mouseUp(MouseButtonEvent event) override;

        void mouseEnter() override;

        void mouseLeave() override;

        void unfocus() override;

        void keyDown(KeyEvent event) override;

        Observable<ButtonClickEvent>& onClick();

        const std::string& getLabel() const;

        void setLabel(const std::string& newLabel);

        void setLabel(std::string&& newLabel);

        void setNormalSprite(const std::shared_ptr<Sprite>& sprite);

        void setPressedSprite(const std::shared_ptr<Sprite>& sprite);

    private:
        void activateButton(const ButtonClickEvent& event);
    };
}

#endif
