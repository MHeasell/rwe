#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <deque>
#include <functional>
#include <optional>
#include <queue>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/DiscreteRect.h>
#include <rwe/GameSimulation.h>
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
#include <rwe/pathfinding/PathFindingService.h>

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
    };
    struct NormalCursorMode
    {
        bool selecting{false};
    };

    using CursorMode = boost::variant<AttackCursorMode, NormalCursorMode>;

    enum class ImpactType
    {
        Normal,
        Water
    };

    class GameScene : public SceneManager::Scene
    {
    public:
        class UnitCommandDispacher : public boost::static_visitor<>
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

            void operator()(const PlayerUnitCommand::Stop&)
            {
                scene->stopUnit(unit);
            }
        };

        class PlayerCommandDispatcher : public boost::static_visitor<>
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
                boost::apply_visitor(dispatcher, c.command);
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

        SceneContext sceneContext;

        RenderService renderService;
        UiRenderService uiRenderService;

        GameSimulation simulation;

        MovementClassCollisionService collisionService;

        UnitFactory unitFactory;

        PathFindingService pathFindingService;
        UnitBehaviorService unitBehaviorService;
        CobExecutionService cobExecutionService;
        PlayerCommandService playerCommandService;

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

        bool occupiedGridVisible{false};
        bool pathfindingVisualisationVisible{false};
        bool movementClassGridVisible{false};

        bool healthBarsVisible{false};

        CursorMode cursorMode{NormalCursorMode()};

        std::deque<std::optional<GameSceneTimeAction>> actions;

        std::vector<PlayerCommand> localPlayerCommandBuffer;

    public:
        GameScene(
            const SceneContext& sceneContext,
            RenderService&& renderService,
            UiRenderService&& uiRenderService,
            GameSimulation&& simulation,
            MovementClassCollisionService&& collisionService,
            UnitDatabase&& unitDatabase,
            MeshService&& meshService,
            PlayerId localPlayerId);

        void init() override;

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void update() override;

        void spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position);

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

    private:
        std::optional<UnitId> getUnitUnderCursor() const;

        Vector2f screenToClipSpace(Point p) const;

        Point getMousePosition() const;

        std::optional<UnitId> getFirstCollidingUnit(const Ray3f& ray) const;

        std::optional<Vector3f> getMouseTerrainCoordinate() const;

        void localPlayerIssueUnitOrder(UnitId unitId, const UnitOrder& order);

        void localPlayerEnqueueUnitOrder(UnitId unitId, const UnitOrder& order);

        void localPlayerStopUnit(UnitId unitId);

        void issueUnitOrder(UnitId unitId, const UnitOrder& order);

        void enqueueUnitOrder(UnitId unitId, const UnitOrder& order);

        void stopUnit(UnitId unitId);

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

        bool hasPlayerCommands() const;

        void processPlayerCommands(const std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>& commands);

        template <typename T>
        void delay(SceneTimeDelta interval, T&& f)
        {
            actions.push_back(GameSceneTimeAction(sceneTime + interval, std::forward<T>(f)));
        }
    };
}

#endif
