#pragma once

#include <random>
#include <rwe/collections/SimpleVectorMap.h>
#include <rwe/collections/VectorMap.h>
#include <rwe/game/MovementClassCollisionService.h>
#include <rwe/game/PlayerColorIndex.h>
#include <rwe/geometry/BoundingBox3x.h>
#include <rwe/pathfinding/PathFindingService.h>
#include <rwe/sim/FeatureDefinition.h>
#include <rwe/sim/FeatureId.h>
#include <rwe/sim/GameHash.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/MapFeature.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClassId.h>
#include <rwe/sim/OccupiedGrid.h>
#include <rwe/sim/PlayerId.h>
#include <rwe/sim/Projectile.h>
#include <rwe/sim/ProjectileId.h>
#include <rwe/sim/SimAxis.h>
#include <rwe/sim/UnitDefinition.h>
#include <rwe/sim/UnitModelDefinition.h>
#include <rwe/sim/UnitState.h>
#include <set>
#include <unordered_map>

namespace rwe
{
    enum class GamePlayerStatus
    {
        Alive,
        Dead
    };

    enum class GamePlayerType
    {
        Human,
        Computer
    };

    struct GamePlayerInfo
    {
        std::optional<std::string> name;
        GamePlayerType type;
        PlayerColorIndex color;
        GamePlayerStatus status;
        std::string side;

        Metal metal;
        Energy energy;

        Metal maxMetal;
        Energy maxEnergy;

        Metal startingMetal;
        Energy startingEnergy;

        bool metalStalled{false};
        bool energyStalled{false};

        Metal desiredMetalConsumptionBuffer{0};
        Energy desiredEnergyConsumptionBuffer{0};

        Metal previousDesiredMetalConsumptionBuffer{0};
        Energy previousDesiredEnergyConsumptionBuffer{0};

        Metal actualMetalConsumptionBuffer{0};
        Energy actualEnergyConsumptionBuffer{0};

        Metal metalProductionBuffer{0};
        Energy energyProductionBuffer{0};

        bool addResourceDelta(const Energy& apparentEnergy, const Metal& apparentMetal, const Energy& actualEnergy, const Metal& actualMetal);
        bool recordAndCheckDesire(const Energy& energy);
        bool recordAndCheckDesire(const Metal& metal);
        void acceptResource(const Energy& energy);
        void acceptResource(const Metal& metal);
    };

    struct PathRequest
    {
        UnitId unitId;

        bool operator==(const PathRequest& rhs) const;

        bool operator!=(const PathRequest& rhs) const;
    };

    struct WinStatusWon
    {
        PlayerId winner;
    };
    struct WinStatusDraw
    {
    };
    struct WinStatusUndecided
    {
    };
    using WinStatus = std::variant<WinStatusWon, WinStatusDraw, WinStatusUndecided>;

    struct FireWeaponEvent
    {
        std::string weaponType;
        /**
         * The number of this shot within the weapon's current burst.
         * If this is the first shot of the burst, it will be 0.
         */
        int shotNumber;
        SimVector firePoint;
    };

    struct UnitArrivedEvent
    {
        UnitId unitId;
    };

    struct UnitActivatedEvent
    {
        UnitId unitId;
    };

    struct UnitDeactivatedEvent
    {
        UnitId unitId;
    };

    struct UnitCompleteEvent
    {
        UnitId unitId;
    };

    struct UnitSpawnedEvent
    {
        UnitId unitId;
    };

    struct UnitDiedEvent
    {
        UnitId unitId;
        std::string unitType;
        SimVector position;
        enum class DeathType
        {
            NormalExploded,
            WaterExploded,
            Deleted,
        };
        DeathType deathType;
    };

    struct UnitStartedBuildingEvent
    {
        UnitId unitId;
    };

    struct EmitParticleFromPieceEvent
    {
        enum class SfxType
        {
            LightSmoke,
            BlackSmoke,
            Wake1,
        };

        SfxType sfxType;
        UnitId unitId;
        std::string pieceName;
    };

    struct ProjectileSpawnedEvent
    {
        ProjectileId projectileId;
    };

    struct ProjectileDiedEvent
    {
        ProjectileId projectileId;
        std::string weaponType;
        SimVector position;

        enum class DeathType
        {
            OutOfBounds,
            NormalImpact,
            WaterImpact,
            EndOfLife,
        };
        DeathType deathType;
    };

    using GameEvent = std::variant<
        FireWeaponEvent,
        UnitArrivedEvent,
        UnitActivatedEvent,
        UnitDeactivatedEvent,
        UnitCompleteEvent,
        EmitParticleFromPieceEvent,
        UnitSpawnedEvent,
        UnitDiedEvent,
        UnitStartedBuildingEvent,
        ProjectileSpawnedEvent,
        ProjectileDiedEvent>;


    struct UnitInfo
    {
        const UnitId id;
        UnitState* const state;
        const UnitDefinition* const definition;

        UnitInfo(UnitId id, UnitState* state, const UnitDefinition* definition)
            : id(id), state(state), definition(definition) {}
    };

    struct ConstUnitInfo
    {
        const UnitId id;
        const UnitState* const state;
        const UnitDefinition* const definition;

        ConstUnitInfo(UnitId id, const UnitState* state, const UnitDefinition* definition)
            : id(id), state(state), definition(definition) {}

        ConstUnitInfo(UnitInfo unitInfo)
            : id(unitInfo.id), state(unitInfo.state), definition(unitInfo.definition) {}
    };

    enum class ImpactType
    {
        Normal,
        Water
    };

    struct GameSimulation
    {
        std::minstd_rand rng;

        WinStatus gameStatus{WinStatusUndecided()};

        MapTerrain terrain;

        std::unordered_map<std::string, UnitDefinition> unitDefinitions;

        SimpleVectorMap<FeatureDefinition, FeatureDefinitionIdTag> featureDefinitions;
        std::unordered_map<std::string, FeatureDefinitionId> featureNameIndex;

        std::unordered_map<std::string, UnitModelDefinition> unitModelDefinitions;

        std::unordered_map<std::string, CobScript> unitScriptDefinitions;

        std::unordered_map<std::string, WeaponDefinition> weaponDefinitions;

        std::unordered_map<MovementClassId, MovementClass> movementClassDefinitions;
        MovementClassCollisionService movementClassCollisionService;

        PathFindingService pathFindingService;

        OccupiedGrid occupiedGrid;
        std::set<UnitId> flyingUnitsSet;

        Grid<unsigned char> metalGrid;

        std::vector<GamePlayerInfo> players;

        VectorMap<MapFeature, FeatureIdTag> features;

        VectorMap<UnitState, UnitIdTag> units;

        VectorMap<Projectile, ProjectileIdTag> projectiles;

        std::deque<PathRequest> pathRequests;

        std::deque<UnitId> unitCreationRequests;

        GameTime gameTime{0};

        std::vector<GameEvent> events;

        explicit GameSimulation(MapTerrain&& terrain, MovementClassCollisionService&& movementClassCollisionService, unsigned char surfaceMetal);

        FeatureId addFeature(MapFeature&& newFeature);

        FeatureId addFeature(FeatureDefinitionId featureType, int heightmapX, int heightmapZ);

        PlayerId addPlayer(const GamePlayerInfo& info);

        std::optional<UnitId> trySpawnUnit(const std::string& unitType, PlayerId owner, const SimVector& position, std::optional<SimAngle> rotation);

        /**
         * Returns true if the unit was really added, false otherwise.
         * A unit might not be added because it violates collision constraints.
         */
        std::optional<UnitId> tryAddUnit(UnitState&& unit);

        /**
         * Returns true if a unit with the given movementclass attributes
         * could be built at given location on the map -- i.e. it is valid terrain
         * for the unit and it is not occupied by something else.
         */
        bool canBeBuiltAt(const MovementClass& mc, unsigned int x, unsigned int y) const;

        DiscreteRect computeFootprintRegion(const SimVector& position, unsigned int footprintX, unsigned int footprintZ) const;

        DiscreteRect computeFootprintRegion(const SimVector& position, const UnitDefinition::MovementCollisionInfo& collisionInfo) const;

        bool isCollisionAt(const DiscreteRect& rect) const;

        bool isCollisionAt(const GridRegion& region) const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        bool isYardmapBlocked(unsigned int x, unsigned int y, const Grid<YardMapCell>& yardMap, bool open) const;

        bool isAdjacentToObstacle(const DiscreteRect& rect) const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void enableShading(UnitId unitId, const std::string& name);

        void disableShading(UnitId unitId, const std::string& name);

        UnitState& getUnitState(UnitId id);

        const UnitState& getUnitState(UnitId id) const;

        UnitInfo getUnitInfo(UnitId id);

        ConstUnitInfo getUnitInfo(UnitId id) const;

        std::optional<std::reference_wrapper<UnitState>> tryGetUnitState(UnitId id);

        std::optional<std::reference_wrapper<const UnitState>> tryGetUnitState(UnitId id) const;

        bool unitExists(UnitId id) const;

        MapFeature& getFeature(FeatureId id);

        const MapFeature& getFeature(FeatureId id) const;

        GamePlayerInfo& getPlayer(PlayerId player);

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        void moveObject(UnitId unitId, const std::string& name, SimAxis axis, SimScalar position, SimScalar speed);

        void moveObjectNow(UnitId unitId, const std::string& name, SimAxis axis, SimScalar position);

        void turnObject(UnitId unitId, const std::string& name, SimAxis axis, SimAngle angle, SimScalar speed);

        void turnObjectNow(UnitId unitId, const std::string& name, SimAxis axis, SimAngle angle);

        void spinObject(UnitId unitId, const std::string& name, SimAxis axis, SimScalar speed, SimScalar acceleration);

        void stopSpinObject(UnitId unitId, const std::string& name, SimAxis axis, SimScalar deceleration);

        bool isPieceMoving(UnitId unitId, const std::string& name, SimAxis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, SimAxis axis) const;

        std::optional<SimVector> intersectLineWithTerrain(const Line3x<SimScalar>& line) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        void requestPath(UnitId unitId);

        Projectile createProjectileFromWeapon(PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget, std::optional<UnitId> targetUnit);

        Projectile createProjectileFromWeapon(PlayerId owner, const std::string& weaponType, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget, std::optional<UnitId> targetUnit);

        void spawnProjectile(PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget, std::optional<UnitId> targetUnit);

        WinStatus computeWinStatus() const;

        bool addResourceDelta(const UnitId& unitId, const Energy& apparentEnergy, const Metal& apparentMetal, const Energy& actualEnergy, const Metal& actualMetal);
        bool addResourceDelta(const UnitId& unitId, const Energy& energy, const Metal& metal);

        bool trySetYardOpen(const UnitId& unitId, bool open);

        void emitBuggerOff(const UnitId& unitId);

        void tellToBuggerOff(const UnitId& unitId, const DiscreteRect& rect);

        GameHash computeHash() const;

        void activateUnit(UnitId unitId);

        void deactivateUnit(UnitId unitId);

        void quietlyKillUnit(UnitId unitId);

        Matrix4x<SimScalar> getUnitPieceLocalTransform(UnitId unitId, const std::string& pieceName) const;

        Matrix4x<SimScalar> getUnitPieceTransform(UnitId unitId, const std::string& pieceName) const;

        SimVector getUnitPiecePosition(UnitId unitId, const std::string& pieceName) const;

        void setBuildStance(UnitId unitId, bool value);

        void setYardOpen(UnitId unitId, bool value);

        void setBuggerOff(UnitId unitId, bool value);

        MovementClass getAdHocMovementClass(const UnitDefinition::MovementCollisionInfo& info) const;

        std::pair<unsigned int, unsigned int> getFootprintXZ(const UnitDefinition::MovementCollisionInfo& info) const;

        BoundingBox3x<SimScalar> createBoundingBox(const UnitState& unit) const;

        void killUnit(UnitId unitId);

        void applyDamage(UnitId unitId, unsigned int damagePoints);

        void applyDamageInRadius(const SimVector& position, SimScalar radius, const Projectile& projectile);

        void doProjectileImpact(const Projectile& projectile, ImpactType impactType);

        void updateProjectiles();

        void killPlayer(PlayerId playerId);

        void processVictoryCondition();

        void updateResources();

        void trySpawnFeature(const std::string& featureType, const SimVector& position, SimAngle rotation);

        void deleteDeadUnits();

        void deleteDeadProjectiles();

        void spawnNewUnits();

        void tick();

        std::optional<FeatureDefinitionId> tryGetFeatureDefinitionId(const std::string& featureName) const;

        const FeatureDefinition& getFeatureDefinition(FeatureDefinitionId featureDefinitionId) const;
    };
}
