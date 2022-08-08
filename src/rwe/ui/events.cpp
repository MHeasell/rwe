#include "events.h"

namespace rwe
{
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

    ActivateMessage::Type sourceToType(const ButtonClickEvent::Source& s)
    {
        switch (s)
        {
            case ButtonClickEvent::Source::RightMouseButton:
                return ActivateMessage::Type::Secondary;
            default:
                return ActivateMessage::Type::Primary;
        }
    }
}
