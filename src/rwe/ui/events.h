#pragma once

#include <rwe/events.h>
#include <string>
#include <variant>

namespace rwe
{
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

    ActivateMessage::Type sourceToType(const ButtonClickEvent::Source& s);
}
