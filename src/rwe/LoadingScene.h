#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/SceneManager.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/TextureService.h>
#include <rwe/CursorService.h>

namespace rwe
{
    class LoadingScene : public SceneManager::Scene
    {
    private:
        std::unique_ptr<UiPanel> panel;

        TextureService* textureService;
        CursorService* cursor;

    public:
        LoadingScene(TextureService* textureService, CursorService* cursor);

        void init() override;

        void render(GraphicsContext& context) override;
    };
}

#endif
