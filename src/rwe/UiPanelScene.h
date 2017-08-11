#ifndef RWE_MAINMENUSCENE_H
#define RWE_MAINMENUSCENE_H

#include <rwe/SceneManager.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/TextureService.h>

namespace rwe
{
    class UiPanelScene : public SceneManager::Scene
    {
    private:
        UiPanel panel;

    public:
        explicit UiPanelScene(UiPanel&& panel);

        void render(GraphicsContext& context) override;
    };
}

#endif
