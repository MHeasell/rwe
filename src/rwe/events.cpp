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

    MouseWheelEvent::MouseWheelEvent(int x, int y) : x(x), y(y)
    {
    }

    GroupMessage::GroupMessage(
        const std::string& topic,
        unsigned int group,
        const std::string& controlName,
        const ControlMessage& message)
        : topic(topic), group(group), controlName(controlName), message(message)
    {
    }

    GroupMessage::GroupMessage(
        const std::string& topic,
        unsigned int group,
        const std::string& controlName,
        const ScrollPositionMessage& message)
        : topic(topic), group(group), controlName(controlName), message(message)
    {
    }

    GroupMessage::GroupMessage(
        const std::string& topic, unsigned int group, const std::string& controlName, const ScrollUpMessage& message)
        : topic(topic), group(group), controlName(controlName), message(message)
    {
    }

    GroupMessage::GroupMessage(
        const std::string& topic, unsigned int group, const std::string& controlName, const ScrollDownMessage& message)
        : topic(topic), group(group), controlName(controlName), message(message)
    {
    }
}
