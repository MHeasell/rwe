#ifndef RWE_LOADINGSCENE_H
#define RWE_LOADINGSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameScene.h>
#include <rwe/LoadingNetworkService.h>
#include <rwe/MapFeatureService.h>
#include <rwe/SceneContext.h>
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
    struct PlayerControllerTypeHuman
    {
    };
    struct PlayerControllerTypeComputer
    {
    };
    struct PlayerControllerTypeNetwork
    {
        std::string host;
        std::string port;
    };

    using PlayerControllerType = boost::variant<PlayerControllerTypeHuman, PlayerControllerTypeComputer, PlayerControllerTypeNetwork>;

    class IsHumanVisitor : public boost::static_visitor<bool>
    {
    public:
        bool operator()(const PlayerControllerTypeHuman&) const { return true; }
        bool operator()(const PlayerControllerTypeComputer&) const { return false; }
        bool operator()(const PlayerControllerTypeNetwork&) const { return false; }
    };

    class GetNetworkAddressVisitor : public boost::static_visitor<std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>>>
    {
    public:
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeHuman&) const { return std::nullopt; }
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeComputer&) const { return std::nullopt; }
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeNetwork& p) const { return std::make_pair(std::cref(p.host), std::cref(p.port)); }
    };

    struct PlayerInfo
    {
        PlayerControllerType controller;
        std::string side;
        unsigned int color;
    };

    struct GameParameters
    {
        std::string mapName;
        unsigned int schemaIndex;
        std::array<std::optional<PlayerInfo>, 10> players;
        std::string localNetworkInterface{"0.0.0.0"};
        std::string localNetworkPort{"1337"};

        GameParameters(const std::string& mapName, unsigned int schemaIndex);
    };

    class LoadingScene : public SceneManager::Scene
    {
    private:
        SceneContext sceneContext;

        std::unique_ptr<UiPanel> panel;

        MapFeatureService* featureService;

        UiRenderService scaledUiRenderService;
        UiRenderService nativeUiRenderService;

        AudioService::LoopToken bgm;

        GameParameters gameParameters;

        std::vector<UiLightBar*> bars;

        LoadingNetworkService networkService;

    public:
        LoadingScene(
            const SceneContext& sceneContext,
            MapFeatureService* featureService,
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
