#pragma once

#include <boost/range/adaptor/map.hpp>
#include <boost/version.hpp>
#include <deque>
#include <fstream>
#include <functional>
#include <optional>
#include <queue>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/GameNetworkService.h>
#include <rwe/InGameSoundsInfo.h>
#include <rwe/MeshService.h>
#include <rwe/PlayerCommand.h>
#include <rwe/PlayerCommandService.h>
#include <rwe/RenderService.h>
#include <rwe/SceneContext.h>
#include <rwe/SceneManager.h>
#include <rwe/SceneTime.h>
#include <rwe/TextureService.h>
#include <rwe/UiRenderService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/UnitFactory.h>
#include <rwe/UnitSoundType.h>
#include <rwe/Viewport.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/cob/CobExecutionService.h>
#include <rwe/grid/DiscreteRect.h>
#include <rwe/observable/BehaviorSubject.h>
#include <rwe/pathfinding/PathFindingService.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/OccupiedGrid.h>
#include <rwe/sim/PlayerId.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/Unit.h>
#include <rwe/sim/UnitBehaviorService.h>
#include <rwe/sim/UnitId.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/ui/UiPanel.h>
#include <variant>

namespace rwe
{
    struct GameSceneTimeAction
    {
        using Time = SceneTime;
        Time triggerTime;
        std::function<void()> callback;

        GameSceneTimeAction(Time triggerTime, const std::function<void()>& callback)
            : triggerTime(triggerTime), callback(callback)
        {
        }

        GameSceneTimeAction(Time triggerTime, std::function<void()>&& callback)
            : triggerTime(triggerTime), callback(std::move(callback))
        {
        }
    };

    struct AttackCursorMode
    {
        bool operator==(const AttackCursorMode& /*rhs*/) const { return true; }
        bool operator!=(const AttackCursorMode& /*rhs*/) const { return false; }
    };

    struct MoveCursorMode
    {
        bool operator==(const MoveCursorMode& /*rhs*/) const { return true; }
        bool operator!=(const MoveCursorMode& /*rhs*/) const { return false; }
    };

    struct GuardCursorMode
    {
        bool operator==(const GuardCursorMode& /*rhs*/) const { return true; }
        bool operator!=(const GuardCursorMode& /*rhs*/) const { return false; }
    };

    struct NormalCursorMode
    {
        struct SelectingState
        {
            SceneTime startTime;
            Point startPosition;
            explicit SelectingState(SceneTime startTime, const Point& startPosition) : startTime(startTime), startPosition(startPosition) {}
            SelectingState(int x, int y) : startPosition(x, y) {}
            bool operator==(const SelectingState& rhs) const
            {
                return startTime == rhs.startTime && startPosition == rhs.startPosition;
            }
            bool operator!=(const SelectingState& rhs) const
            {
                return !(rhs == *this);
            }
        };
        struct DraggingMinimapState
        {
            bool operator==(const DraggingMinimapState& /*rhs*/) const { return true; }
            bool operator!=(const DraggingMinimapState& /*rhs*/) const { return false; }
        };
        struct UpState
        {
            bool operator==(const UpState& /*rhs*/) const { return true; }
            bool operator!=(const UpState& /*rhs*/) const { return false; }
        };
        using State = std::variant<SelectingState, DraggingMinimapState, UpState>;

        State state{UpState()};

        bool operator==(const NormalCursorMode& rhs) const
        {
            return state == rhs.state;
        }

        bool operator!=(const NormalCursorMode& rhs) const
        {
            return !(rhs == *this);
        }
    };

    struct BuildCursorMode
    {
        std::string unitType;

        bool operator==(const BuildCursorMode& rhs) const
        {
            return unitType == rhs.unitType;
        }

        bool operator!=(const BuildCursorMode& rhs) const
        {
            return !(rhs == *this);
        }
    };

    using CursorMode = std::variant<AttackCursorMode, MoveCursorMode, GuardCursorMode, BuildCursorMode, NormalCursorMode>;

    enum class ImpactType
    {
        Normal,
        Water
    };

    struct UnitGuiInfo
    {
        enum class Section
        {
            Build,
            Orders,
        };

        Section section;
        int currentBuildPage;
    };

    struct HoverBuildInfo
    {
        DiscreteRect rect;
        bool isValid;
    };

    class GameScene : public SceneManager::Scene
    {
    public:
        static inline const SimScalar SecondsPerTick = SimScalar(SceneManager::TickInterval) / 1000_ss;

        static constexpr int GuiSizeLeft = 128;
        static constexpr int GuiSizeRight = 0;
        static constexpr int GuiSizeTop = 32;
        static constexpr int GuiSizeBottom = 32;

    private:
        static const int UnitSelectChannel = 0;

        static const int reservedChannelsCount = 1;

        /**
         * Speed the camera pans via the arrow keys
         * in world units/second.
         */
        static constexpr float CameraPanSpeed = 1000.0f;

        static const Rectangle2f minimapViewport;

        SceneContext sceneContext;

        std::unique_ptr<Subscription> audioSub = sceneContext.audioService->getChannelFinished().subscribe([this](int channel) { onChannelFinished(channel); });

        Viewport worldViewport;

        std::unique_ptr<PlayerCommandService> playerCommandService;

        RenderService worldRenderService;
        UiRenderService worldUiRenderService;
        UiRenderService chromeUiRenderService;

        GameSimulation simulation;

        MapTerrainGraphics terrainGraphics;

        MovementClassCollisionService collisionService;

        UnitDatabase unitDatabase;
        UnitFactory unitFactory;

        std::unique_ptr<GameNetworkService> gameNetworkService;

        PathFindingService pathFindingService;
        CobExecutionService cobExecutionService;
        UnitBehaviorService unitBehaviorService;

        std::shared_ptr<Sprite> minimap;
        std::shared_ptr<SpriteSeries> minimapDots;
        std::shared_ptr<Sprite> minimapDotHighlight;
        Rectangle2f minimapRect;

        std::unique_ptr<UiPanel> currentPanel;
        std::optional<std::unique_ptr<UiPanel>> nextPanel;

        InGameSoundsInfo sounds;

        std::shared_ptr<SpriteSeries> guiFont;

        PlayerId localPlayerId;

        SceneTime sceneTime{0};

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

        bool leftShiftDown{false};
        bool rightShiftDown{false};

        std::optional<UnitId> hoveredUnit;
        std::unordered_set<UnitId> selectedUnits;

        std::optional<HoverBuildInfo> hoverBuildInfo;

        bool occupiedGridVisible{false};
        bool pathfindingVisualisationVisible{false};
        bool movementClassGridVisible{false};
        bool cursorTerrainDotVisible{false};

        bool healthBarsVisible{false};

        BehaviorSubject<CursorMode> cursorMode{NormalCursorMode()};

        std::deque<std::optional<GameSceneTimeAction>> actions;

        std::vector<PlayerCommand> localPlayerCommandBuffer;
        bool commandWasQueued{false};

        BehaviorSubject<UnitFireOrders> fireOrders{UnitFireOrders::HoldFire};
        BehaviorSubject<bool> onOff{false};

        UiFactory uiFactory;

        std::unordered_map<UnitId, UnitGuiInfo> unitGuiInfos;

        std::unordered_map<UnitId, std::unordered_map<std::string, int>> unconfirmedBuildQueueDelta;

        std::vector<std::pair<GameTime, GameHash>> gameHashes;

        std::optional<std::ofstream> stateLogStream;

        bool showDebugWindow{false};
        char unitSpawnText[20]{""};
        int unitSpawnPlayer{0};

        std::mutex playingUnitChannelsLock;
        std::unordered_set<int> playingUnitChannels;

        std::vector<Explosion> explosions;

        int millisecondsBuffer{0};

    public:
        GameScene(
            const SceneContext& sceneContext,
            std::unique_ptr<PlayerCommandService>&& playerCommandService,
            MeshDatabase&& meshDatabase,
            CabinetCamera camera,
            SharedTextureHandle unitTextureAtlas,
            std::vector<SharedTextureHandle>&& unitTeamTextureAtlases,
            UiRenderService&& worldUiRenderService,
            UiRenderService&& chromeUiRenderService,
            GameSimulation&& simulation,
            MapTerrainGraphics&& terrainGraphics,
            MovementClassCollisionService&& collisionService,
            UnitDatabase&& unitDatabase,
            MeshService&& meshService,
            std::unique_ptr<GameNetworkService>&& gameNetworkService,
            const std::shared_ptr<Sprite>& minimap,
            const std::shared_ptr<SpriteSeries>& minimapDots,
            const std::shared_ptr<Sprite>& minimapDotHighlight,
            InGameSoundsInfo sounds,
            const std::shared_ptr<SpriteSeries>& guiFont,
            PlayerId localPlayerId,
            TdfBlock* audioLookup,
            std::optional<std::ofstream>&& stateLogStream);

        void init() override;

        void render() override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void onMouseWheel(MouseWheelEvent event) override;

        void update(int millisecondsElapsed) override;

        std::optional<UnitId> spawnUnit(const std::string& unitType, PlayerId owner, const SimVector& position, std::optional<const std::reference_wrapper<SimAngle>> rotation);

        std::optional<std::reference_wrapper<Unit>> spawnCompletedUnit(const std::string& unitType, PlayerId owner, const SimVector& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void moveObject(UnitId unitId, const std::string& name, Axis axis, SimScalar position, SimScalar speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, SimScalar position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, SimAngle angle, SimScalar speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, SimAngle angle);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        Matrix4x<SimScalar> getUnitPieceLocalTransform(UnitId unitId, const std::string& pieceName) const;

        SimVector getUnitPiecePosition(UnitId unitId, const std::string& pieceName) const;

        GameTime getGameTime() const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        void playUiSound(const AudioService::SoundHandle& sound);

        void playNotificationSound(const PlayerId& playerId, const AudioService::SoundHandle& sound);

        void playUnitNotificationSound(const PlayerId& playerId, const std::string& unitType, UnitSoundType soundType);

        void playSoundAt(const Vector3f& position, const AudioService::SoundHandle& sound);

        void playWeaponStartSound(const Vector3f& position, const std::string& weaponType);

        void playWeaponImpactSound(const Vector3f& position, const std::string& weaponType, ImpactType impactType);

        void spawnWeaponImpactExplosion(const Vector3f& position, const std::string& weaponType, ImpactType impactType);

        DiscreteRect computeFootprintRegion(const SimVector& position, int footprintX, int footprintZ) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        GameSimulation& getSimulation();

        const GameSimulation& getSimulation() const;

        void doProjectileImpact(const Projectile& projectile, ImpactType impactType);

        void createLightSmoke(const Vector3f& position);

        void createWeaponSmoke(const Vector3f& position);

        void activateUnit(UnitId unitId);
        void deactivateUnit(UnitId unitId);

        void modifyBuildQueue(UnitId unitId, const std::string& unitType, int count);

        void setBuildStance(UnitId unitId, bool value);

        void setYardOpen(UnitId unitId, bool value);

        void quietlyKillUnit(UnitId unitId);

        void setBuggerOff(UnitId unitId, bool value);

        void onChannelFinished(int channel);

    private:
        static Matrix4f worldToMinimapMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect);

        static Matrix4f minimapToWorldMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect);

        void tryTickGame();

        std::optional<UnitId> getUnitUnderCursor() const;

        Vector2f screenToWorldClipSpace(Point p) const;

        bool isCursorOverMinimap() const;

        bool isCursorOverWorld() const;

        Point getMousePosition() const;

        std::optional<UnitId> getFirstCollidingUnit(const Ray3f& ray) const;

        /**
         * Returns a value if the given ray intersects this unit
         * for the purposes of unit selection.
         * The value returned is the distance along the ray
         * where the intersection occurred.
         */
        std::optional<float> selectionIntersect(const Unit& unit, const CollisionMesh& mesh, const Ray3f& ray) const;

        std::optional<SimVector> getMouseTerrainCoordinate() const;

        void localPlayerIssueUnitOrder(UnitId unitId, const UnitOrder& order);

        void localPlayerEnqueueUnitOrder(UnitId unitId, const UnitOrder& order);

        void localPlayerStopUnit(UnitId unitId);

        void localPlayerSetFireOrders(UnitId unitId, UnitFireOrders orders);

        void localPlayerSetOnOff(UnitId unitId, bool on);

        void localPlayerModifyBuildQueue(UnitId unitId, const std::string& unitType, int count);

        void issueUnitOrder(UnitId unitId, const UnitOrder& order);

        void enqueueUnitOrder(UnitId unitId, const UnitOrder& order);

        void stopUnit(UnitId unitId);

        void setFireOrders(UnitId unitId, UnitFireOrders orders);

        bool isShiftDown() const;

        void handleEscapeDown();

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        std::optional<std::reference_wrapper<Unit>> tryGetUnit(UnitId id);

        std::optional<std::reference_wrapper<const Unit>> tryGetUnit(UnitId id) const;

        GamePlayerInfo& getPlayer(PlayerId player);

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        bool isEnemy(UnitId id) const;

        bool isFriendly(UnitId id) const;

        void updateProjectiles();

        void updateExplosions();

        void applyDamageInRadius(const SimVector& position, SimScalar radius, const Projectile& projectile);

        void applyDamage(UnitId unitId, int damagePoints);

        void deleteDeadUnits();

        void deleteDeadProjectiles();

        void spawnNewUnits();

        BoundingBox3x<SimScalar> createBoundingBox(const Unit& unit) const;

        void killUnit(UnitId unitId);

        void killPlayer(PlayerId playerId);

        void processActions();

        void processPlayerCommands(const std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>& commands);

        void processPlayerCommand(const PlayerCommand& playerCommand);

        void processUnitCommand(const PlayerUnitCommand& unitCommand);

        template <typename T>
        void delay(SceneTime interval, T&& f)
        {
            actions.emplace_back(GameSceneTimeAction(sceneTime + interval, std::forward<T>(f)));
        }

        void renderMinimap();

        void renderWorld();

        void renderDebugWindow();

        void renderUnitOrders(UnitId unitId, bool drawLines);

        void renderBuildBoxes(const Unit& unit, const Color& color);

        void attachOrdersMenuEventHandlers();

        void onMessage(const std::string& message, ActivateMessage::Type mode);

        bool matchesWithSidePrefix(const std::string& suffix, const std::string& value) const;

        /**
         * If there is exactly one unit selected, returns the unit ID.
         * Otherwise, returns nothing.
         */
        std::optional<UnitId> getSingleSelectedUnit() const;

        void selectUnitsInBandbox(const DiscreteRect& box);

        void toggleUnitSelection(const UnitId& unitId);

        void toggleUnitSelection(const std::unordered_set<UnitId>& units);

        void selectAdditionalUnit(const UnitId& unitId);

        void replaceUnitSelection(const UnitId& unitId);

        void replaceUnitSelection(const std::unordered_set<UnitId>& units);

        void deselectUnit(const UnitId& unitId);

        void clearUnitSelection();

        void onSelectedUnitsChanged();

        const UnitGuiInfo& getGuiInfo(const UnitId& unitId) const;

        void setNextPanel(std::unique_ptr<UiPanel>&& panel);

        void refreshBuildGuiTotal(UnitId unitId, const std::string& unitType);

        void updateUnconfirmedBuildQueueDelta(UnitId unitId, const std::string& unitType, int count);

        int getUnconfirmedBuildQueueCount(UnitId unitId, const std::string& unitType) const;

        std::unique_ptr<UiPanel> createBuildPanel(const std::string& guiname, const std::vector<GuiEntry>& panelDefinition, const std::unordered_map<std::string, int>& totals);

        template <typename T>
        std::optional<std::reference_wrapper<T>> findWithSidePrefix(UiPanel& p, const std::string& name)
        {
            for (const auto& side : (*sceneContext.sideData | boost::adaptors::map_values))
            {
                auto control = p.find<T>(side.namePrefix + name);
                if (control)
                {
                    return control;
                }
            }

            return std::nullopt;
        }

        bool leftClickMode() const;

        void spawnExplosion(const Vector3f& position, const std::string& gaf, const std::string& anim);

        void spawnSmoke(const Vector3f& position, const std::string& gaf, const std::string& anim, ExplosionFinishTime duration, GameTime frameDuration);
    };
}
