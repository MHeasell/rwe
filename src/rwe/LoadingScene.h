#pragma once

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameScene.h>
#include <rwe/LoadingNetworkService.h>
#include <rwe/MapTerrainGraphics.h>
#include <rwe/PlayerColorIndex.h>
#include <rwe/SceneContext.h>
#include <rwe/SceneManager.h>
#include <rwe/SideData.h>
#include <rwe/TextureService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/io/featuretdf/FeatureDefinition.h>
#include <rwe/io/ota/ota.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/render/TextureArrayRegion.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/sim/SimVector.h>
#include <rwe/ui/UiFactory.h>
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

    using PlayerControllerType = std::variant<PlayerControllerTypeHuman, PlayerControllerTypeComputer, PlayerControllerTypeNetwork>;

    class IsHumanVisitor
    {
    public:
        bool operator()(const PlayerControllerTypeHuman&) const { return true; }
        bool operator()(const PlayerControllerTypeComputer&) const { return false; }
        bool operator()(const PlayerControllerTypeNetwork&) const { return false; }
    };

    class IsComputerVisitor
    {
    public:
        bool operator()(const PlayerControllerTypeHuman&) const { return false; }
        bool operator()(const PlayerControllerTypeComputer&) const { return true; }
        bool operator()(const PlayerControllerTypeNetwork&) const { return false; }
    };

    class GetNetworkAddressVisitor
    {
    public:
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeHuman&) const { return std::nullopt; }
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeComputer&) const { return std::nullopt; }
        std::optional<std::pair<std::reference_wrapper<const std::string>, std::reference_wrapper<const std::string>>> operator()(const PlayerControllerTypeNetwork& p) const { return std::make_pair(std::cref(p.host), std::cref(p.port)); }
    };

    struct PlayerInfo
    {
        std::optional<std::string> name;
        PlayerControllerType controller;
        std::string side;
        PlayerColorIndex color;

        Metal metal;
        Energy energy;
    };

    struct GameParameters
    {
        std::string mapName;
        unsigned int schemaIndex;
        std::array<std::optional<PlayerInfo>, 10> players;
        std::string localNetworkPort{"1337"};
        std::optional<std::string> stateLogFile;

        GameParameters(const std::string& mapName, unsigned int schemaIndex);
    };

    class LoadingScene : public SceneManager::Scene
    {
    private:
        SceneContext sceneContext;

        std::unique_ptr<UiPanel> panel;

        UiRenderService scaledUiRenderService;
        UiRenderService nativeUiRenderService;

        TdfBlock* audioLookup;

        AudioService::LoopToken bgm;

        GameParameters gameParameters;

        std::vector<UiLightBar*> bars;

        LoadingNetworkService networkService;

        UiFactory uiFactory;

    public:
        LoadingScene(
            const SceneContext& sceneContext,
            TdfBlock* audioLookup,
            AudioService::LoopToken&& bgm,
            GameParameters gameParameters);

        void init() override;

        void render() override;

    private:
        static unsigned int computeMidpointHeight(const Grid<unsigned char>& heightmap, std::size_t x, std::size_t y);

        std::unique_ptr<GameScene> createGameScene(const std::string& mapName, unsigned int schemaIndex);


        struct LoadMapResult
        {
            MapTerrain terrain;
            unsigned char surfaceMetal;
            std::vector<MapFeature> features;
            MapTerrainGraphics terrainGraphics;
        };

        LoadMapResult loadMap(const std::unordered_map<std::string, FeatureDefinition>& featuresMap, const std::string& mapName, const rwe::OtaRecord& ota, unsigned int schemaIndex);

        std::vector<TextureArrayRegion> getTileTextures(TntArchive& tnt);

        Grid<std::size_t> getMapData(TntArchive& tnt);

        Grid<unsigned char> getHeightGrid(const Grid<TntTileAttributes>& attrs) const;

        std::vector<FeatureDefinition> getFeatures(const std::unordered_map<std::string, FeatureDefinition>& featuresMap, TntArchive& tnt);

        MapFeature createFeature(const SimVector& pos, const FeatureDefinition& definition);

        SimVector computeFeaturePosition(const MapTerrain& terrain, const FeatureDefinition& featureDefinition, std::size_t x, std::size_t y) const;

        const SideData& getSideData(const std::string& side) const;

        std::pair<UnitDatabase, MeshDatabase> createUnitDatabase(MeshService& meshService);

        void preloadSound(UnitDatabase& db, const std::string& soundName);

        void preloadSound(UnitDatabase& db, const std::optional<std::string>& soundName);

        std::optional<AudioService::SoundHandle> lookUpSound(const std::string& key);

        std::optional<std::vector<std::vector<GuiEntry>>> loadBuilderGui(const std::string& unitName);
    };
}
