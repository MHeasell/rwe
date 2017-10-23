#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameScene.h>
#include <rwe/MapFeatureService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/tnt/TntArchive.h>
#include <rwe/ui/UiLightBar.h>
#include <rwe/ui/UiPanel.h>
#include "ota.h"
#include "SideData.h"

namespace rwe
{
    struct PlayerInfo
    {
        std::string side;
    };

    struct GameParameters
    {
        std::string mapName;
        unsigned int schemaIndex;
        std::array<boost::optional<PlayerInfo>, 10> players;

        GameParameters(const std::string& mapName, unsigned int schemaIndex);
    };

    class LoadingScene : public SceneManager::Scene
    {
    private:
        std::unique_ptr<UiPanel> panel;

        AbstractVirtualFileSystem* vfs;
        TextureService* textureService;
        CursorService* cursor;
        GraphicsContext* graphics;
        MapFeatureService* featureService;
        const ColorPalette* palette;
        SceneManager* sceneManager;
        const std::unordered_map<std::string, SideData>* sideData;

        AudioService::LoopToken bgm;

        GameParameters gameParameters;

        std::vector<UiLightBar*> bars;

    public:
        LoadingScene(
            AbstractVirtualFileSystem* vfs,
            TextureService* textureService,
            CursorService* cursor,
            GraphicsContext* graphics,
            MapFeatureService* featureService,
            const ColorPalette* palette,
            SceneManager* sceneManager,
            const std::unordered_map<std::string, SideData>* sideData,
            AudioService::LoopToken&& bgm,
            GameParameters gameParameters);

        void init() override;

        void render(GraphicsContext& context) override;

    private:
        static unsigned int computeMidpointHeight(const Grid<unsigned char>& heightmap, std::size_t x, std::size_t y);

        GameScene createGameScene(const std::string& mapName, unsigned int schemaIndex);

        MapTerrain createMapTerrain(const std::string& mapName, const rwe::OtaRecord& ota, unsigned int schemaIndex);

        std::vector<TextureRegion> getTileTextures(TntArchive& tnt);

        Grid<std::size_t> getMapData(TntArchive& tnt);

        Grid<unsigned char> getHeightGrid(const Grid<TntTileAttributes>& attrs) const;

        std::vector<FeatureDefinition> getFeatures(TntArchive& tnt);

        MapFeature createFeature(const Vector3f& pos, const FeatureDefinition& definition);

        Vector3f computeFeaturePosition(const MapTerrain& terrain, const FeatureDefinition& featureDefinition, std::size_t x, std::size_t y) const;

        ShaderProgramHandle loadShader(const std::string& vertexShaderName, const std::string& fragmentShaderName, const std::vector<AttribMapping>& attribs);

        std::string slurpFile(const std::string& filename);

        const SideData& getSideData(const std::string& side) const;
    };
}

#endif
