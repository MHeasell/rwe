#include "events.h"

namespace rwe
{
    MouseButtonEvent::MouseButtonEvent(int x, int y, MouseButtonEvent::MouseButton button) : x(x), y(y), button(button)
    {
    }

    KeyEvent::KeyEvent(int keyCode) : keyCode(keyCode)
    {
    }

    MouseMoveEvent::MouseMoveEvent(int x, int y) : x(x), y(y)
    {
    }
}
