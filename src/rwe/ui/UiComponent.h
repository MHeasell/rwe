#ifndef RWE_UICOMPONENT_H
#define RWE_UICOMPONENT_H

#include <rwe/GraphicsContext.h>
#include <rwe/events.h>

namespace rwe
{
    class UiComponent
    {
    protected:
        int posX;
        int posY;

        unsigned int sizeX;
        unsigned int sizeY;

    public:
        UiComponent(int posX, int posY, unsigned int sizeX, unsigned int sizeY)
            : posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY)
        {
        }

        UiComponent(const UiComponent& c) = default;
        UiComponent& operator=(const UiComponent& c) = default;
        UiComponent(UiComponent&& c) = default;
        UiComponent& operator=(UiComponent&& c) = default;

        virtual ~UiComponent() = default;

        unsigned int getWidth() { return sizeX; }

        unsigned int getHeight() { return sizeY; }

        int getX() { return posX; }
        int getY() { return posY; }

        virtual bool mouseDown(MouseButtonEvent /*event*/) { return false; }

        virtual bool mouseUp(MouseButtonEvent /*event*/) { return false; }

        virtual void mouseEnter() {}

        virtual void mouseLeave() {}

        virtual bool keyDown(KeyEvent /*event*/) { return false; }

        virtual bool keyUp(KeyEvent /*event*/) { return false; }

        virtual void focus() {}

        virtual void unfocus() {}

        virtual void render(GraphicsContext& /*graphics*/) const {}

        bool contains(int x, int y)
        {
            auto minX = posX;
            auto maxX = posX + static_cast<int>(sizeX) - 1;
            auto minY = posY;
            auto maxY = posY + static_cast<int>(sizeY) - 1;
            return x >= minX && x <= maxX && y >= minY && y <= maxY;
        }
    };
}

#endif
