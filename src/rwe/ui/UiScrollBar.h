#ifndef RWE_UISCROLLBAR_H
#define RWE_UISCROLLBAR_H

#include <memory>
#include <rwe/SpriteSeries.h>
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

        float scrollBarPercent{0.5f};

        float scrollPercent{0.0f};

        bool barGrabbed{false};

        bool upArrowPressed{false};

        bool downArrowPressed{false};

        bool backgroundPressed{false};

        int mouseY;

        int mouseDownY;
        float mouseDownScrollPercent;

    public:
        UiScrollBar(
            int posX,
            int posY,
            unsigned int sizeX,
            unsigned int sizeY,
            std::shared_ptr<SpriteSeries> sprites);

        void render(GraphicsContext& context) const override;

        void mouseDown(MouseButtonEvent event) override;

        void mouseUp(MouseButtonEvent event) override;

        void mouseMove(MouseMoveEvent event) override;

        void update(float dt) override;

        void uiMessage(const GroupMessage& message) override;

    private:
        void drawScrollBox(GraphicsContext& context, float x, float y, float height) const;

        void drawScrollBackground(GraphicsContext& graphics, float x, float y, float height) const;

        ScrollBoxInfo getScrollBoxInfo() const;

        /**
         * Converts the input control-relative pixel coordinate
         * to a scroll percentage from 0 to 1.
         */
        float toScrollPercent(float pos);
    };
}

#endif
