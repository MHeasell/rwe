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
}
