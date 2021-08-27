#pragma once

#include <memory>
#include <rwe/observable/Subject.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    class UiScrollBar : public UiComponent
    {
    private:
        struct ScrollBoxInfo
        {
            float pos;
            float size;
            float range;
        };

        std::shared_ptr<SpriteSeries> sprites;

        /**
         * The percentage of the scrollbar that is occupied by the scroll box,
         * from 0.0 to 1.0.
         */
        float scrollBarPercent{1.0f};

        /**
         * The percentage through the contents that we have scrolled,
         * from 0.0 to 1.0.
         */
        float scrollPercent{0.0f};

        bool barGrabbed{false};

        bool upArrowPressed{false};

        bool downArrowPressed{false};

        bool backgroundPressed{false};

        int mouseY;

        int mouseDownY;
        float mouseDownScrollPercent;

        Subject<bool> scrollUpSubject;
        Subject<bool> scrollDownSubject;
        Subject<float> scrollChangedSubject;

    public:
        UiScrollBar(
            int posX,
            int posY,
            int sizeX,
            int sizeY,
            std::shared_ptr<SpriteSeries> sprites);

        void render(UiRenderService& context) const override;

        void mouseDown(MouseButtonEvent event) override;

        void mouseUp(MouseButtonEvent event) override;

        void mouseMove(MouseMoveEvent event) override;

        void mouseWheel(MouseWheelEvent event) override;

        void update(float dt) override;

        void uiMessage(const GroupMessage& message) override;

        Observable<float>& scrollChanged();

        const Observable<float>& scrollChanged() const;

        Observable<bool>& scrollUp();

        const Observable<bool>& scrollUp() const;

        Observable<bool>& scrollDown();

        const Observable<bool>& scrollDown() const;

        float getScrollBarPercent() const;

    private:
        void drawScrollBox(UiRenderService& context, float x, float y, float height) const;

        void drawScrollBackground(UiRenderService& graphics, float x, float y, float height) const;

        ScrollBoxInfo getScrollBoxInfo() const;

        /**
         * Converts the input control-relative pixel coordinate
         * to a scroll percentage from 0 to 1.
         */
        float toScrollPercent(float pos);

        /**
         * Gets the effective size of the scroll box
         * as a percentage of the scroll bar.
         * This is the size that will actually be used for rendering.
         * This imposes a minimum to ensure that the box is never too small.
         */
        float getEffectiveScrollBarPercent() const;
    };
}
