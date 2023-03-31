#pragma once

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/LoadingNetworkService.h>
#include <rwe/SceneContext.h>
#include <rwe/TextureService.h>
#include <rwe/game/GameScene.h>
#include <rwe/game/MapTerrainGraphics.h>
#include <rwe/game/PlayerColorIndex.h>
#include <rwe/game/UnitDatabase.h>
#include <rwe/io/featuretdf/FeatureTdf.h>
#include <rwe/io/ota/ota.h>
#include <rwe/io/sidedatatdf/SideData.h>
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

    class LoadingScene : public Scene
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
        static int computeMidpointHeight(const Grid<unsigned char>& heightmap, int x, int y);

        std::unique_ptr<GameScene> createGameScene(const std::string& mapName, unsigned int schemaIndex);


        struct LoadMapResult
        {
            MapTerrain terrain;
            unsigned char surfaceMetal;
            std::vector<std::pair<Point, std::string>> features;
            MapTerrainGraphics terrainGraphics;
        };

        LoadMapResult loadMap(const std::string& mapName, const rwe::OtaRecord& ota, unsigned int schemaIndex);

        std::vector<TextureArrayRegion> getTileTextures(TntArchive& tnt);

        Grid<std::size_t> getMapData(TntArchive& tnt);

        Grid<unsigned char> getHeightGrid(const Grid<TntTileAttributes>& attrs) const;

        std::vector<FeatureTdf> getFeatures(const std::unordered_map<std::string, FeatureTdf>& featuresMap, TntArchive& tnt);

        SimVector computeFeaturePosition(const MapTerrain& terrain, const FeatureDefinition& featureDefinition, int x, int y) const;

        const SideData& getSideData(const std::string& side) const;

        struct DataMaps
        {
            std::unordered_map<std::string, UnitDefinition> unitDefinitions;
            std::unordered_map<std::string, UnitModelDefinition> modelDefinitions;
            std::unordered_map<std::string, WeaponDefinition> weaponDefinitions;
            std::unordered_map<MovementClassId, MovementClass> movementClassDefinitions;
            SimpleVectorMap<FeatureDefinition, FeatureDefinitionIdTag> featureDefinitions;
            std::unordered_map<std::string, FeatureDefinitionId> featureNameIndex;
        };

        std::tuple<UnitDatabase, MeshDatabase, DataMaps, MovementClassCollisionService> createUnitDatabase(const MapTerrain& terrain, MeshService& meshService, const std::unordered_set<std::string>& requiredFeatures);

        void preloadSound(MeshDatabase& meshDb, const std::string& soundName);

        void preloadSound(MeshDatabase& meshDb, const std::optional<std::string>& soundName);

        std::optional<AudioService::SoundHandle> lookUpSound(const std::string& key);

        std::optional<std::vector<std::vector<GuiEntry>>> loadBuilderGui(const std::string& unitName);

        void loadFeature(MeshService& meshService, MeshDatabase& meshDatabase, const std::unordered_map<std::string, FeatureTdf>& tdfs, DataMaps& dataMaps, const std::string& initialFeatureName);
        void loadFeatureMedia(MeshService& meshService, std::unordered_map<std::string, UnitModelDefinition>& modelDefinitions, MeshDatabase& meshDatabase, const FeatureTdf& tdf);
    };
}
