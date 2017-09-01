#include "UiPanelScene.h"

namespace rwe
{
    UiPanelScene::UiPanelScene(UiPanel&& _panel)
        : panel(std::move(_panel)),
          camera(panel.getWidth(), panel.getHeight())
    {
    }

    void UiPanelScene::render(GraphicsContext& context)
    {
        context.applyCamera(camera);
        panel.render(context);
    }

    void UiPanelScene::onMouseDown(MouseButtonEvent event)
    {
        panel.mouseDown(event);
    }

    void UiPanelScene::onMouseUp(MouseButtonEvent event)
    {
        panel.mouseUp(event);
    }

    void UiPanelScene::onMouseMove(MouseMoveEvent event)
    {
        panel.mouseMove(event);
    }
}
