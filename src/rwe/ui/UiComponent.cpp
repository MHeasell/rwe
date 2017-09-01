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
}
