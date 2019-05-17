#ifndef RWE_EVENTS_H
#define RWE_EVENTS_H

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

    struct ScrollPositionMessage
    {
        float scrollViewportPercent;
        float scrollPosition;
    };

    struct ScrollUpMessage
    {
    };

    struct ScrollDownMessage
    {
    };

    struct ActivateMessage
    {
        enum class Type
        {
            Primary,
            Secondary
        };

        Type type{Type::Primary};
    };

    using ControlMessage = std::variant<ScrollPositionMessage, ScrollUpMessage, ScrollDownMessage, ActivateMessage>;

    struct GroupMessage
    {
        std::string topic;
        unsigned int group;
        std::string controlName;
        ControlMessage message;

        GroupMessage(const std::string& topic, unsigned int group, const std::string& controlName, const ControlMessage& message);

        GroupMessage(const std::string& topic, unsigned int group, const std::string& controlName, const ScrollPositionMessage& message);

        GroupMessage(const std::string& topic, unsigned int group, const std::string& controlName, const ScrollUpMessage& message);

        GroupMessage(const std::string& topic, unsigned int group, const std::string& controlName, const ScrollDownMessage& message);
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

    ActivateMessage::Type sourceToType(const ButtonClickEvent::Source& s);
}

#endif
