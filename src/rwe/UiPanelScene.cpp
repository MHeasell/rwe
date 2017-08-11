#include "UiPanelScene.h"

namespace rwe
{
    UiPanelScene::UiPanelScene(UiPanel&& panel) : panel(std::move(panel))
    {}

    void UiPanelScene::render(GraphicsContext& context)
    {
        panel.render(context);
    }
}
