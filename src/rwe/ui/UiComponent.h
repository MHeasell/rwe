#ifndef RWE_UICOMPONENT_H
#define RWE_UICOMPONENT_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/events.h>
#include <rwe/observable/Subscription.h>
#include <vector>

namespace rwe
{
    class UiComponent
    {
    protected:
        int posX;
        int posY;

        unsigned int sizeX;
        unsigned int sizeY;

    private:
        int lastMouseX{0};
        int lastMouseY{0};

        std::vector<std::unique_ptr<Subscription>> subscriptions;

    public:
        UiComponent(int posX, int posY, unsigned int sizeX, unsigned int sizeY)
            : posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY)
        {
        }

        UiComponent(const UiComponent& c) = default;
        UiComponent& operator=(const UiComponent& c) = default;
        UiComponent(UiComponent&& c) = default;
        UiComponent& operator=(UiComponent&& c) = default;

        virtual ~UiComponent();

        unsigned int getWidth() { return sizeX; }

        unsigned int getHeight() { return sizeY; }

        int getX() { return posX; }
        int getY() { return posY; }

        virtual void mouseDown(MouseButtonEvent /*event*/) {}

        virtual void mouseUp(MouseButtonEvent /*event*/) {}

        virtual void mouseEnter() {}

        virtual void mouseLeave() {}

        virtual void keyDown(KeyEvent /*event*/) {}

        virtual void keyUp(KeyEvent /*event*/) {}

        virtual void focus() {}

        virtual void unfocus() {}

        virtual void render(GraphicsContext& /*graphics*/) const {}

        virtual void mouseMove(MouseMoveEvent event);

        virtual void update(float /*dt*/) {};

        bool contains(int x, int y)
        {
            auto minX = posX;
            auto maxX = posX + static_cast<int>(sizeX) - 1;
            auto minY = posY;
            auto maxY = posY + static_cast<int>(sizeY) - 1;
            return x >= minX && x <= maxX && y >= minY && y <= maxY;
        }

        void addSubscription(std::unique_ptr<Subscription>&& s);
    };
}

#endif
