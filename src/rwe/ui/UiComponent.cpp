#include "UiComponent.h"

namespace rwe
{

    void UiComponent::mouseMove(MouseMoveEvent event)
    {
        if (contains(lastMouseX, lastMouseY) && !contains(event.x, event.y))
        {
            mouseLeave();
        }
        else if (!contains(lastMouseX, lastMouseY) && contains(event.x, event.y))
        {
            mouseEnter();
        }

        lastMouseX = event.x;
        lastMouseY = event.y;
    }

    void UiComponent::addSubscription(std::unique_ptr<Subscription>&& s)
    {
        subscriptions.push_back(std::move(s));
    }

    UiComponent::~UiComponent()
    {
        for (auto& s : subscriptions)
        {
            s->unsubscribe();
        }
    }

    const std::string& UiComponent::getName() const
    {
        return name;
    }

    void UiComponent::setName(const std::string& newName)
    {
        name = newName;
    }

    void UiComponent::setName(std::string&& newName)
    {
        name = std::move(newName);
    }

    int UiComponent::getGroup() const
    {
        return group;
    }

    void UiComponent::setGroup(int newGroup)
    {
        group = newGroup;
    }

    Observable<const ControlMessage&>& UiComponent::messages()
    {
        return messagesSubject;
    }

    const Observable<const ControlMessage&>& UiComponent::messages() const
    {
        return messagesSubject;
    }
}
