#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <rwe/AudioService.h>
#include <rwe/DiscreteRect.h>
#include <rwe/Energy.h>
#include <rwe/Grid.h>
#include <rwe/Metal.h>
#include <rwe/MovementClass.h>
#include <rwe/MovementClassId.h>
#include <rwe/PlayerId.h>
#include <rwe/SelectionMesh.h>
#include <rwe/SimAngle.h>
#include <rwe/SimScalar.h>
#include <rwe/SimVector.h>
#include <rwe/UnitFireOrders.h>
#include <rwe/UnitMesh.h>
#include <rwe/UnitOrder.h>
#include <rwe/UnitWeapon.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/geometry/BoundingBox3f.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/pathfinding/UnitPath.h>
#include <variant>

namespace rwe
{
    struct PathFollowingInfo
    {
        UnitPath path;
        GameTime pathCreationTime;
        std::vector<SimVector>::const_iterator currentWaypoint;
        explicit PathFollowingInfo(UnitPath&& path, GameTime creationTime)
            : path(std::move(path)), pathCreationTime(creationTime), currentWaypoint(this->path.waypoints.begin()) {}
    };

    using MovingStateGoal = std::variant<SimVector, DiscreteRect>;

    struct MovingState
    {
        MovingStateGoal destination;
        std::optional<PathFollowingInfo> path;
        bool pathRequested;
    };

    struct IdleState
    {
    };

    struct UnitCreationStatusPending
    {
    };

    struct UnitCreationStatusDone
    {
        UnitId unitId;
    };

    struct UnitCreationStatusFailed
    {
    };

    using UnitCreationStatus = std::variant<UnitCreationStatusPending, UnitCreationStatusFailed, UnitCreationStatusDone>;
    struct CreatingUnitState
    {
        std::string unitType;
        PlayerId owner;
        SimVector position;
        UnitCreationStatus status{UnitCreationStatusPending()};
    };

    struct BuildingState
    {
        UnitId targetUnit;
        std::optional<SimVector> nanoParticleOrigin;
    };

    using UnitState = std::variant<IdleState, MovingState, CreatingUnitState, BuildingState>;

    struct FactoryStateIdle
    {
    };

    struct FactoryStateCreatingUnit
    {
        std::string unitType;
        PlayerId owner;
        SimVector position;
        UnitCreationStatus status{UnitCreationStatusPending()};
    };

    struct FactoryStateBuilding
    {
        // the vector is the origin of nano particles
        std::optional<std::pair<UnitId, std::optional<SimVector>>> targetUnit;
    };

    using FactoryState = std::variant<FactoryStateIdle, FactoryStateCreatingUnit, FactoryStateBuilding>;

    UnitOrder createMoveOrder(const SimVector& destination);

    UnitOrder createAttackOrder(UnitId target);

    UnitOrder createAttackGroundOrder(const SimVector& target);

    enum class YardMapCell
    {
        GroundPassableWhenOpen,
        WaterPassableWhenOpen,
        GroundNoFeature,
        GroundGeoPassableWhenOpen,
        Geo,
        Ground,
        GroundPassableWhenClosed,
        Water,
        GroundPassable,
        WaterPassable,
        Passable
    };

    bool isPassable(YardMapCell cell, bool yardMapOpen);

    class Unit
    {
    public:
        enum class LifeState
        {
            Alive,
            Dead,
        };

    public:
        std::string name;
        std::string unitType;
        UnitMesh mesh;
        SimVector position;
        std::unique_ptr<CobEnvironment> cobEnvironment;
        SelectionMesh selectionMesh;
        std::optional<AudioService::SoundHandle> selectionSound;
        std::optional<AudioService::SoundHandle> okSound;
        std::optional<AudioService::SoundHandle> arrivedSound;
        std::optional<AudioService::SoundHandle> buildSound;
        std::optional<AudioService::SoundHandle> completeSound;
        std::optional<AudioService::SoundHandle> activateSound;
        std::optional<AudioService::SoundHandle> deactivateSound;
        PlayerId owner;

        /**
         * The height of the unit. Typically computed from the mesh.
         */
        SimScalar height;

        /**
         * Anticlockwise rotation of the unit around the Y axis in radians.
         * The other two axes of rotation are normally determined
         * by the normal of the terrain the unit is standing on.
         */
        SimAngle rotation{0};


        /**
         * Rate at which the unit turns in world angular units/tick.
         */
        SimScalar turnRate;

        /**
         * Rate at which the unit is travelling forwards in game units/tick.
         */
        SimScalar currentSpeed{0};

        /**
         * Maximum speed the unit can travel forwards in game units/tick.
         */
        SimScalar maxSpeed;

        /**
         * Speed at which the unit accelerates in game units/tick.
         */
        SimScalar acceleration;

        /**
         * Speed at which the unit brakes in game units/tick.
         */
        SimScalar brakeRate;

        /** The angle we are trying to steer towards. */
        SimAngle targetAngle{0};

        /** The speed we are trying to accelerate/decelerate to */
        SimScalar targetSpeed{0};

        std::optional<MovementClassId> movementClass;

        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;

        /** If true, the unit is considered a commander for victory conditions. */
        bool commander;

        unsigned int hitPoints{0};
        unsigned int maxHitPoints;

        LifeState lifeState{LifeState::Alive};

        std::deque<UnitOrder> orders;
        UnitState behaviourState;

        /**
         * State we remember related to the current order.
         * This is cleared every time an order is completed.
         * Right now this is just a unit ID for build orders.
         */
        std::optional<UnitId> buildOrderUnitId;

        bool inBuildStance{false};
        bool yardOpen{false};

        std::optional<Grid<YardMapCell>> yardMap;

        /**
         * True if the unit attempted to move last frame
         * and its movement was limited (or prevented entirely) by a collision.
         */
        bool inCollision{false};

        std::array<std::optional<UnitWeapon>, 3> weapons;

        bool canAttack;
        bool canMove;
        bool canGuard;

        std::optional<UnitWeapon> explosionWeapon;

        UnitFireOrders fireOrders{UnitFireOrders::FireAtWill};

        bool builder;

        unsigned int buildTime;
        Energy energyCost;
        Metal metalCost;

        unsigned int buildTimeCompleted{0};

        unsigned int workerTimePerTick;

        SimScalar buildDistance;

        bool onOffable;
        bool activateWhenBuilt;

        Energy energyUse;
        Metal metalUse;

        Energy energyMake;
        Metal metalMake;

        bool activated{false};
        bool isSufficientlyPowered{false};

        bool hideDamage{false};
        bool showPlayerName{false};

        Energy energyProductionBuffer{0};
        Metal metalProductionBuffer{0};
        Energy previousEnergyConsumptionBuffer{0};
        Metal previousMetalConsumptionBuffer{0};
        Energy energyConsumptionBuffer{0};
        Metal metalConsumptionBuffer{0};

        bool isMobile;

        std::deque<std::pair<std::string, int>> buildQueue;
        FactoryState factoryState;

        Metal extractsMetal;

        static SimAngle toRotation(const SimVector& direction);

        static SimVector toDirection(SimAngle rotation);

        Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, SelectionMesh&& selectionMesh);

        bool isBeingBuilt() const;

        unsigned int getBuildPercentLeft() const;

        float getPreciseCompletePercent() const;

        struct BuildCostInfo
        {
            unsigned int workerTime;
            Energy energyCost;
            Metal metalCost;
        };

        BuildCostInfo getBuildCostInfo(unsigned int buildTimeContribution);

        bool addBuildProgress(unsigned int buildTimeContribution);

        bool isCommander() const;

        void moveObject(const std::string& pieceName, Axis axis, SimScalar targetPosition, SimScalar speed);

        void moveObjectNow(const std::string& pieceName, Axis axis, SimScalar targetPosition);

        void turnObject(const std::string& pieceName, Axis axis, SimAngle targetAngle, SimScalar speed);

        void turnObjectNow(const std::string& pieceName, Axis axis, SimAngle targetAngle);

        void spinObject(const std::string& pieceName, Axis axis, SimScalar speed, SimScalar acceleration);

        void stopSpinObject(const std::string& pieceName, Axis axis, SimScalar deceleration);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        bool isTurnInProgress(const std::string& pieceName, Axis axis) const;

        /**
         * Returns a value if the given ray intersects this unit
         * for the purposes of unit selection.
         * The value returned is the distance along the ray
         * where the intersection occurred.
         */
        std::optional<float> selectionIntersect(const Ray3f& ray) const;

        bool isOwnedBy(PlayerId playerId) const;

        bool isDead() const;

        void markAsDead();

        void finishBuilding();

        void clearOrders();

        void replaceOrders(const std::deque<UnitOrder>& newOrders);

        void addOrder(const UnitOrder& order);

        void setWeaponTarget(unsigned int weaponIndex, UnitId target);
        void setWeaponTarget(unsigned int weaponIndex, const SimVector& target);
        void clearWeaponTarget(unsigned int weaponIndex);
        void clearWeaponTargets();

        Matrix4x<SimScalar> getTransform() const;
        Matrix4x<SimScalar> getInverseTransform() const;

        bool isSelectableBy(PlayerId player) const;

        void activate();

        void deactivate();

        MovementClass getAdHocMovementClass() const;

        Metal getMetalMake() const;
        Energy getEnergyMake() const;
        Metal getMetalUse() const;
        Energy getEnergyUse() const;

        void addEnergyDelta(const Energy& energy);
        void addMetalDelta(const Metal& metal);

        void resetResourceBuffers();

        void modifyBuildQueue(const std::string& buildUnitType, int count);

        std::unordered_map<std::string, int> getBuildQueueTotals() const;

        int getBuildQueueTotal(const std::string& unitType) const;

        std::optional<std::pair<UnitId, SimVector>> getActiveNanolatheTarget() const;
    };
}
