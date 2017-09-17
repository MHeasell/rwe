#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/SceneManager.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/TextureService.h>
#include <rwe/CursorService.h>
#include <rwe/ui/UiLightBar.h>

namespace rwe
{
    class LoadingScene : public SceneManager::Scene
    {
    private:
        std::unique_ptr<UiPanel> panel;

        TextureService* textureService;
        CursorService* cursor;

        std::vector<UiLightBar*> bars;

    public:
        LoadingScene(TextureService* textureService, CursorService* cursor);

        void init() override;

        void render(GraphicsContext& context) override;
    };
}

#endif
