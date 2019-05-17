#ifndef RWE_GAMESIMULATION_H
#define RWE_GAMESIMULATION_H

#include <rwe/Explosion.h>
#include <rwe/FeatureId.h>
#include <rwe/GameTime.h>
#include <rwe/LaserProjectile.h>
#include <rwe/MapFeature.h>
#include <rwe/MapTerrain.h>
#include <rwe/OccupiedGrid.h>
#include <rwe/PlayerColorIndex.h>
#include <rwe/PlayerId.h>
#include <rwe/Unit.h>
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
        Metal maxMetal;

        Energy energy;
        Energy maxEnergy;

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

    struct GameSimulation
    {
        WinStatus gameStatus{WinStatusUndecided()};

        MapTerrain terrain;

        OccupiedGrid occupiedGrid;

        std::vector<GamePlayerInfo> players;

        std::unordered_map<FeatureId, MapFeature> features;

        UnitId nextUnitId{0};

        FeatureId nextFeatureId{0};

        std::unordered_map<UnitId, Unit> units;

        std::vector<std::optional<LaserProjectile>> lasers;

        std::vector<std::optional<Explosion>> explosions;

        std::deque<PathRequest> pathRequests;

        GameTime gameTime{0};

        explicit GameSimulation(MapTerrain&& terrain);

        FeatureId addFeature(MapFeature&& newFeature);

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

        DiscreteRect computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const;

        bool isCollisionAt(const DiscreteRect& rect) const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        bool isAdjacentToObstacle(const DiscreteRect& rect, UnitId self) const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void enableShading(UnitId unitId, const std::string& name);

        void disableShading(UnitId unitId, const std::string& name);

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        bool unitExists(UnitId id) const;

        MapFeature& getFeature(FeatureId id);

        const MapFeature& getFeature(FeatureId id) const;

        GamePlayerInfo& getPlayer(PlayerId player);

        const GamePlayerInfo& getPlayer(PlayerId player) const;

        void moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle, float speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle);

        void spinObject(UnitId unitId, const std::string& name, Axis axis, float speed, float acceleration);

        void stopSpinObject(UnitId unitId, const std::string& name, Axis axis, float deceleration);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        std::optional<UnitId> getFirstCollidingUnit(const Ray3f& ray) const;

        std::optional<Vector3f> intersectLineWithTerrain(const Line3f& line) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        void requestPath(UnitId unitId);

        LaserProjectile createProjectileFromWeapon(PlayerId owner, const UnitWeapon& weapon, const Vector3f& position, const Vector3f& direction);

        void spawnLaser(PlayerId owner, const UnitWeapon& weapon, const Vector3f& position, const Vector3f& direction);

        void spawnExplosion(const Vector3f& position, const std::shared_ptr<SpriteSeries>& animation);

        void spawnSmoke(const Vector3f& position, const std::shared_ptr<SpriteSeries>& animation);

        WinStatus computeWinStatus() const;

        bool addResourceDelta(const UnitId& unitId, const Energy& apparentEnergy, const Metal& apparentMetal, const Energy& actualEnergy, const Metal& actualMetal);
        bool addResourceDelta(const UnitId& unitId, const Energy& energy, const Metal& metal);
    };
}

#endif
