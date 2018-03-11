#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameScene.h>
#include <rwe/MapFeatureService.h>
#include <rwe/SceneManager.h>
#include <rwe/SideData.h>
#include <rwe/TextureService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/ViewportService.h>
#include <rwe/ota.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/ui/UiLightBar.h>
#include <rwe/ui/UiPanel.h>

namespace rwe
{
    struct PlayerInfo
    {
        enum class Controller
        {
            Human,
            Computer
        };

        Controller controller;
        std::string side;
        unsigned int color;
    };

    struct GameParameters
    {
        std::string mapName;
        unsigned int schemaIndex;
        std::array<std::optional<PlayerInfo>, 10> players;

        GameParameters(const std::string& mapName, unsigned int schemaIndex);
    };

    class LoadingScene : public SceneManager::Scene
    {
    private:
        std::unique_ptr<UiPanel> panel;

        AbstractVirtualFileSystem* vfs;
        TextureService* textureService;
        AudioService* audioService;
        CursorService* cursor;
        GraphicsContext* graphics;
        ShaderService* shaders;
        MapFeatureService* featureService;
        const ColorPalette* palette;
        const ColorPalette* guiPalette;
        SceneManager* sceneManager;
        SdlContext* sdl;
        const std::unordered_map<std::string, SideData>* sideData;
        ViewportService* viewportService;

        UiRenderService scaledUiRenderService;
        UiRenderService nativeUiRenderService;

        AudioService::LoopToken bgm;

        GameParameters gameParameters;

        std::vector<UiLightBar*> bars;

    public:
        LoadingScene(
            AbstractVirtualFileSystem* vfs,
            TextureService* textureService,
            AudioService* audioService,
            CursorService* cursor,
            GraphicsContext* graphics,
            ShaderService* shaders,
            MapFeatureService* featureService,
            const ColorPalette* palette,
            const ColorPalette* guiPalette,
            SceneManager* sceneManager,
            SdlContext* sdl,
            const std::unordered_map<std::string, SideData>* sideData,
            ViewportService* viewportService,
            AudioService::LoopToken&& bgm,
            GameParameters gameParameters);

        void init() override;

        void render(GraphicsContext& context) override;

    private:
        static unsigned int computeMidpointHeight(const Grid<unsigned char>& heightmap, std::size_t x, std::size_t y);

        std::unique_ptr<GameScene> createGameScene(const std::string& mapName, unsigned int schemaIndex);

        GameSimulation createInitialSimulation(const std::string& mapName, const rwe::OtaRecord& ota, unsigned int schemaIndex);

        std::vector<TextureRegion> getTileTextures(TntArchive& tnt);

        Grid<std::size_t> getMapData(TntArchive& tnt);

        Grid<unsigned char> getHeightGrid(const Grid<TntTileAttributes>& attrs) const;

        std::vector<FeatureDefinition> getFeatures(TntArchive& tnt);

        MapFeature createFeature(const Vector3f& pos, const FeatureDefinition& definition);

        Vector3f computeFeaturePosition(const MapTerrain& terrain, const FeatureDefinition& featureDefinition, std::size_t x, std::size_t y) const;

        const SideData& getSideData(const std::string& side) const;

        UnitDatabase createUnitDatabase();

        void preloadSound(UnitDatabase& db, const std::string& soundName);

        void preloadSound(UnitDatabase& db, const std::optional<std::string>& soundName);
    };
}

#endif
