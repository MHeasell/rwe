#pragma once

#include <random>
#include <rwe/GameHash.h>
#include <rwe/PlayerColorIndex.h>
#include <rwe/VectorMap.h>
#include <rwe/sim/FeatureDefinition.h>
#include <rwe/sim/FeatureId.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/MapFeature.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/OccupiedGrid.h>
#include <rwe/sim/Particle.h>
#include <rwe/sim/PlayerId.h>
#include <rwe/sim/Projectile.h>
#include <rwe/sim/ProjectileId.h>
#include <rwe/sim/Unit.h>
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

        Metal maxMetal{0};
        Energy maxEnergy{0};

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

    using GameEvent = std::variant<FireWeaponEvent, UnitArrivedEvent, UnitActivatedEvent, UnitDeactivatedEvent>;

    struct GameSimulation
    {
        std::minstd_rand rng;

        WinStatus gameStatus{WinStatusUndecided()};

        MapTerrain terrain;

        std::unordered_map<std::string, WeaponDefinition> weaponDefinitions;

        OccupiedGrid occupiedGrid;

        Grid<unsigned char> metalGrid;

        std::vector<GamePlayerInfo> players;

        VectorMap<MapFeature, FeatureIdTag> features;

        UnitId nextUnitId{0};

        VectorMap<Unit, UnitIdTag> units;

        VectorMap<Projectile, ProjectileIdTag> projectiles;

        std::deque<PathRequest> pathRequests;

        std::deque<UnitId> unitCreationRequests;

        GameTime gameTime{0};

        std::vector<GameEvent> events;

        explicit GameSimulation(MapTerrain&& terrain, unsigned char surfaceMetal);

        FeatureId addFeature(const FeatureDefinition& featureDefinition, MapFeature&& newFeature);

        PlayerId addPlayer(const GamePlayerInfo& info);

        /**
         * Returns true if the unit was really added, false otherwise.
         * A unit might not be added because it violates collision constraints.
         */
        std::optional<UnitId> tryAddUnit(Unit&& unit);

        /**
         * Returns true if a unit with the given movementclass attributes
         * could be built at given location on the map -- i.e. it is valid terrain
         * for the unit and it is not occupied by something else.
         */
        bool canBeBuiltAt(const MovementClass& mc, unsigned int x, unsigned int y) const;

        DiscreteRect computeFootprintRegion(const SimVector& position, unsigned int footprintX, unsigned int footprintZ) const;

        bool isCollisionAt(const DiscreteRect& rect) const;

        bool isCollisionAt(const GridRegion& region) const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        bool isYardmapBlocked(unsigned int x, unsigned int y, const Grid<YardMapCell>& yardMap, bool open) const;

        bool isAdjacentToObstacle(const DiscreteRect& rect) const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void enableShading(UnitId unitId, const std::string& name);

        void disableShading(UnitId unitId, const std::string& name);

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        std::optional<std::reference_wrapper<Unit>> tryGetUnit(UnitId id);

        std::optional<std::reference_wrapper<const Unit>> tryGetUnit(UnitId id) const;

        bool unitExists(UnitId id) const;

        MapFeature& getFeature(FeatureId id);

        const MapFeature& getFeature(FeatureId id) const;

        GamePlayerInfo& getPlayer(PlayerId player);

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        void moveObject(UnitId unitId, const std::string& name, Axis axis, SimScalar position, SimScalar speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, SimScalar position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, SimAngle angle, SimScalar speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, SimAngle angle);

        void spinObject(UnitId unitId, const std::string& name, Axis axis, SimScalar speed, SimScalar acceleration);

        void stopSpinObject(UnitId unitId, const std::string& name, Axis axis, SimScalar deceleration);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        std::optional<SimVector> intersectLineWithTerrain(const Line3x<SimScalar>& line) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        void requestPath(UnitId unitId);

        Projectile createProjectileFromWeapon(PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget);

        void spawnProjectile(PlayerId owner, const UnitWeapon& weapon, const SimVector& position, const SimVector& direction, SimScalar distanceToTarget);

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
    };
}
