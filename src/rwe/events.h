#ifndef RWE_EVENTS_H
#define RWE_EVENTS_H

#include <boost/variant.hpp>
#include <string>

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

    struct ScrollPositionMessage
    {
        float scrollViewportPercent;
        float scrollPosition;
    };

    struct GroupMessage
    {
        std::string topic;
        unsigned int group;
        std::string controlName;
        boost::variant<ScrollPositionMessage> message;

        GroupMessage(const std::string& topic, unsigned int group, const std::string& controlName, const ScrollPositionMessage& message);
    };

}

#endif
