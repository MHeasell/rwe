#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <boost/range/adaptor/map.hpp>
#include <boost/version.hpp>
#include <deque>
#include <functional>
#include <optional>
#include <queue>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/DiscreteRect.h>
#include <rwe/GameNetworkService.h>
#include <rwe/GameSimulation.h>
#include <rwe/InGameSoundsInfo.h>
#include <rwe/MeshService.h>
#include <rwe/OccupiedGrid.h>
#include <rwe/PlayerCommand.h>
#include <rwe/PlayerCommandService.h>
#include <rwe/PlayerId.h>
#include <rwe/RenderService.h>
#include <rwe/SceneContext.h>
#include <rwe/SceneManager.h>
#include <rwe/SceneTime.h>
#include <rwe/TextureService.h>
#include <rwe/UiRenderService.h>
#include <rwe/Unit.h>
#include <rwe/UnitBehaviorService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/UnitFactory.h>
#include <rwe/UnitId.h>
#include <rwe/ViewportService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/cob/CobExecutionService.h>
#include <rwe/observable/BehaviorSubject.h>
#include <rwe/pathfinding/PathFindingService.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/ui/UiPanel.h>

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

    struct NormalCursorMode
    {
        enum class State
        {
            Selecting,
            DraggingMinimap,
            Up,
        };

        State state{State::Up};

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

    using CursorMode = std::variant<AttackCursorMode, MoveCursorMode, BuildCursorMode, NormalCursorMode>;

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
        unsigned int currentBuildPage;
    };

    struct HoverBuildInfo
    {
        DiscreteRect rect;
        bool isValid;
    };

    class GameScene : public SceneManager::Scene
    {
    public:
        static constexpr float SecondsPerTick = static_cast<float>(SceneManager::TickInterval) / 1000.0f;

        static constexpr int GuiSizeLeft = 128;
        static constexpr int GuiSizeRight = 0;
        static constexpr int GuiSizeTop = 32;
        static constexpr int GuiSizeBottom = 32;

        class UnitCommandDispacher
        {
        private:
            GameScene* scene;
            PlayerId player;
            UnitId unit;

        public:
            UnitCommandDispacher(GameScene* scene, const PlayerId& player, const UnitId& unit)
                : scene(scene), player(player), unit(unit)
            {
            }

            void operator()(const PlayerUnitCommand::IssueOrder& c)
            {
                switch (c.issueKind)
                {
                    case PlayerUnitCommand::IssueOrder::IssueKind::Immediate:
                        scene->issueUnitOrder(unit, c.order);
                        break;
                    case PlayerUnitCommand::IssueOrder::IssueKind::Queued:
                        scene->enqueueUnitOrder(unit, c.order);
                        break;
                }
            }

            void operator()(const PlayerUnitCommand::ModifyBuildQueue& c)
            {
                scene->modifyBuildQueue(unit, c.unitType, c.count);
            }

            void operator()(const PlayerUnitCommand::Stop&)
            {
                scene->stopUnit(unit);
            }

            void operator()(const PlayerUnitCommand::SetFireOrders& c)
            {
                scene->setFireOrders(unit, c.orders);
            }

            void operator()(const PlayerUnitCommand::SetOnOff& c)
            {
                if (c.on)
                {
                    scene->activateUnit(unit);
                }
                else
                {
                    scene->deactivateUnit(unit);
                }
            }
        };

        class PlayerCommandDispatcher
        {
        private:
            GameScene* scene;
            PlayerId playerId;

        public:
            PlayerCommandDispatcher(GameScene* scene, PlayerId playerId) : scene(scene), playerId(playerId)
            {
            }

            void operator()(const PlayerUnitCommand& c)
            {
                UnitCommandDispacher dispatcher(scene, playerId, c.unit);
                std::visit(dispatcher, c.command);
            }
            void operator()(const PlayerPauseGameCommand&)
            {
                // TODO
            }
            void operator()(const PlayerUnpauseGameCommand&)
            {
                // TODO
            }
        };

    private:
        static const unsigned int UnitSelectChannel = 0;

        static const unsigned int reservedChannelsCount = 1;

        /**
         * Speed the camera pans via the arrow keys
         * in world units/second.
         */
        static constexpr float CameraPanSpeed = 1000.0f;

        static const Rectangle2f minimapViewport;

        SceneContext sceneContext;

        ViewportService worldViewport;

        std::unique_ptr<PlayerCommandService> playerCommandService;

        RenderService worldRenderService;
        UiRenderService worldUiRenderService;
        UiRenderService chromeUiRenderService;

        GameSimulation simulation;

        MovementClassCollisionService collisionService;

        UnitFactory unitFactory;

        std::unique_ptr<GameNetworkService> gameNetworkService;

        PathFindingService pathFindingService;
        UnitBehaviorService unitBehaviorService;
        CobExecutionService cobExecutionService;

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
        std::optional<UnitId> selectedUnit;

        std::optional<HoverBuildInfo> hoverBuildInfo;

        bool occupiedGridVisible{false};
        bool pathfindingVisualisationVisible{false};
        bool movementClassGridVisible{false};
        bool cursorTerrainDotVisible{false};

        bool healthBarsVisible{false};

        BehaviorSubject<CursorMode> cursorMode{NormalCursorMode()};

        std::deque<std::optional<GameSceneTimeAction>> actions;

        std::vector<PlayerCommand> localPlayerCommandBuffer;

        BehaviorSubject<UnitFireOrders> fireOrders{UnitFireOrders::HoldFire};
        BehaviorSubject<bool> onOff{false};

        TdfBlock* audioLookup;
        UiFactory uiFactory;

        std::unordered_map<UnitId, UnitGuiInfo> unitGuiInfos;

        std::unordered_map<UnitId, std::unordered_map<std::string, int>> unconfirmedBuildQueueDelta;

    public:
        GameScene(
            const SceneContext& sceneContext,
            std::unique_ptr<PlayerCommandService>&& playerCommandService,
            RenderService&& worldRenderService,
            UiRenderService&& worldUiRenderService,
            UiRenderService&& chromeUiRenderService,
            GameSimulation&& simulation,
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
            TdfBlock* audioLookup);

        void init() override;

        void render() override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void onMouseWheel(MouseWheelEvent event) override;

        void update() override;

        std::optional<UnitId> spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position);

        void spawnCompletedUnit(const std::string& unitType, PlayerId owner, const Vector3f& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle, float speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        GameTime getGameTime() const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        void playSoundOnSelectChannel(const AudioService::SoundHandle& sound);

        void playUnitSound(UnitId unitId, const AudioService::SoundHandle& sound);

        void playSoundAt(const Vector3f& position, const AudioService::SoundHandle& sound);

        DiscreteRect computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        GameSimulation& getSimulation();

        const GameSimulation& getSimulation() const;

        void doLaserImpact(std::optional<LaserProjectile>& laser, ImpactType impactType);

        void createLightSmoke(const Vector3f& position);

        void activateUnit(UnitId unitId);
        void deactivateUnit(UnitId unitId);

        void modifyBuildQueue(UnitId unitId, const std::string& unitType, int count);

        void setBuildStance(UnitId unitId, bool value);

        void setYardOpen(UnitId unitId, bool value);

        void quietlyKillUnit(UnitId unitId);

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

        std::optional<Vector3f> getMouseTerrainCoordinate() const;

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

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        bool isEnemy(UnitId id) const;

        void updateLasers();

        void updateExplosions();

        void applyDamageInRadius(const Vector3f& position, float radius, const LaserProjectile& laser);

        void applyDamage(UnitId unitId, unsigned int damagePoints);

        void deleteDeadUnits();

        BoundingBox3f createBoundingBox(const Unit& unit) const;

        void killUnit(UnitId unitId);

        void killPlayer(PlayerId playerId);

        void processActions();

        void processPlayerCommands(const std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>& commands);

        template <typename T>
        void delay(SceneTime interval, T&& f)
        {
            actions.emplace_back(GameSceneTimeAction(sceneTime + interval, std::forward<T>(f)));
        }

        void renderMinimap();

        void renderWorld();

        void attachOrdersMenuEventHandlers();

        void onMessage(const std::string& message, ActivateMessage::Type mode);

        bool matchesWithSidePrefix(const std::string& suffix, const std::string& value) const;

        void selectUnit(const UnitId& unitId);

        void deselectUnit(const UnitId& unitId);

        void clearUnitSelection();

        const UnitGuiInfo& getGuiInfo(const UnitId& unitId) const;

        void setNextPanel(std::unique_ptr<UiPanel>&& panel);

        void refreshBuildGuiTotal(UnitId unitId, const std::string& unitType);

        void updateUnconfirmedBuildQueueDelta(UnitId unitId, const std::string& unitType, int count);

        int getUnconfirmedBuildQueueCount(UnitId unitId, const std::string& unitType) const;

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
    };
}

#endif
