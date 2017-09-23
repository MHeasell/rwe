#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameScene.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/ui/UiLightBar.h>
#include <rwe/ui/UiPanel.h>

namespace rwe
{
    struct GameParameters
    {
        std::string mapName;
    };

    class LoadingScene : public SceneManager::Scene
    {
    private:
        std::unique_ptr<UiPanel> panel;

        AbstractVirtualFileSystem* vfs;
        TextureService* textureService;
        CursorService* cursor;
        GraphicsContext* graphics;
        const ColorPalette* palette;
        SceneManager* sceneManager;

        AudioService::LoopToken bgm;

        GameParameters gameParameters;

        std::vector<UiLightBar*> bars;

    public:
        LoadingScene(
            AbstractVirtualFileSystem* vfs,
            TextureService* textureService,
            CursorService* cursor,
            GraphicsContext* graphics,
            const ColorPalette* palette,
            SceneManager* sceneManager,
            AudioService::LoopToken&& bgm,
            GameParameters gameParameters);

        void init() override;

        void render(GraphicsContext& context) override;

    private:
        GameScene createGameScene(const std::string& mapName);

        MapTerrain createMapTerrain(const std::string& mapName);

        std::vector<TextureRegion> getTileTextures(TntArchive& tnt);

        Grid<std::size_t> getMapData(TntArchive& tnt);
    };
}

#endif
