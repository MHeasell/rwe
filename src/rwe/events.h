#ifndef RWE_EVENTS_H
#define RWE_EVENTS_H

namespace rwe
{
    struct MouseButtonEvent
    {
        enum class MouseButton
        {
            Left,
            Middle,
            Right
        };
        int x;
        int y;
        MouseButton button;

        MouseButtonEvent(int x, int y, MouseButton button);
    };

    struct KeyEvent
    {
        int keyCode;

        explicit KeyEvent(int keyCode);
    };

    struct MouseMoveEvent
    {
        int x;
        int y;

        explicit MouseMoveEvent(int x, int y);
    };
}

#endif
