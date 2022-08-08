#pragma once

#include <string>
#include <variant>

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

    struct MouseWheelEvent
    {
        int x;
        int y;

        MouseWheelEvent(int x, int y);
    };

    struct ButtonClickEvent
    {
        enum class Source
        {
            LeftMouseButton,
            MiddleMouseButton,
            RightMouseButton,
            Keyboard,
            Other
        };
        Source source;
    };
}
