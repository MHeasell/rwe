#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <rwe/AudioService.h>
#include <rwe/cob/CobEnvironment.h>
#include <rwe/geometry/BoundingBox3f.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/grid/DiscreteRect.h>
#include <rwe/grid/Grid.h>
#include <rwe/pathfinding/UnitPath.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/MovementClassId.h>
#include <rwe/sim/PlayerId.h>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitDefinition.h>
#include <rwe/sim/UnitFireOrders.h>
#include <rwe/sim/UnitMesh.h>
#include <rwe/sim/UnitOrder.h>
#include <rwe/sim/UnitWeapon.h>
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

    struct UnitBehaviorStateFlyingToLandingSpot
    {
        SimVector landingLocation;
    };

    struct UnitBehaviorStateMoving
    {
        MovingStateGoal destination;
        std::optional<PathFollowingInfo> path;
        bool pathRequested;
    };

    struct UnitBehaviorStateIdle
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
    struct UnitBehaviorStateCreatingUnit
    {
        std::string unitType;
        PlayerId owner;
        SimVector position;
        UnitCreationStatus status{UnitCreationStatusPending()};
    };

    struct UnitBehaviorStateBuilding
    {
        UnitId targetUnit;
        std::optional<SimVector> nanoParticleOrigin;
    };

    using UnitBehaviorState = std::variant<
        UnitBehaviorStateIdle,
        UnitBehaviorStateMoving,
        UnitBehaviorStateCreatingUnit,
        UnitBehaviorStateBuilding,
        UnitBehaviorStateFlyingToLandingSpot>;

    struct FactoryBehaviorStateIdle
    {
    };

    struct FactoryBehaviorStateCreatingUnit
    {
        std::string unitType;
        PlayerId owner;
        SimVector position;
        SimAngle rotation;
        UnitCreationStatus status{UnitCreationStatusPending()};
    };

    struct FactoryBehaviorStateBuilding
    {
        // the vector is the origin of nano particles
        std::optional<std::pair<UnitId, std::optional<SimVector>>> targetUnit;
    };

    using FactoryBehaviorState = std::variant<FactoryBehaviorStateIdle, FactoryBehaviorStateCreatingUnit, FactoryBehaviorStateBuilding>;

    UnitOrder createMoveOrder(const SimVector& destination);

    UnitOrder createAttackOrder(UnitId target);

    UnitOrder createAttackGroundOrder(const SimVector& target);

    bool isWater(YardMapCell cell);

    bool isPassable(YardMapCell cell, bool yardMapOpen);

    struct SteeringInfo
    {
        /** The angle we are trying to steer towards. */
        SimAngle targetAngle{0};

        /** The speed we are trying to accelerate/decelerate to */
        SimScalar targetSpeed{0};

        /** True if the unit should attempt to take off into the air */
        bool shouldTakeOff{false};
    };

    struct UnitPhysicsInfoGround
    {
        SteeringInfo steeringInfo;

        /**
         * Rate at which the unit is travelling forwards in game units/tick.
         */
        SimScalar currentSpeed{0};
    };

    struct AirMovementStateFlying
    {
        std::optional<SimVector> targetPosition;

        /** True if the unit should attempt to land at current position. */
        bool shouldLand{false};

        /**
         * Rate at which the unit is moving in game units/tick.
         */
        SimVector currentVelocity{0_ss, 0_ss, 0_ss};
    };

    struct AirMovementStateTakingOff
    {
    };

    struct AirMovementStateLanding
    {
        bool landingFailed{false};
        bool shouldAbort{false};
    };

    using AirMovementState = std::variant<AirMovementStateTakingOff, AirMovementStateFlying, AirMovementStateLanding>;

    struct UnitPhysicsInfoAir
    {
        AirMovementState movementState{AirMovementStateTakingOff()};
    };

    using UnitPhysicsInfo = std::variant<UnitPhysicsInfoGround, UnitPhysicsInfoAir>;

    class UnitState
    {
    public:
        struct LifeStateAlive
        {
        };
        struct LifeStateDead
        {
            bool leaveCorpse;
        };
        using LifeState = std::variant<LifeStateAlive, LifeStateDead>;

    public:
        std::string unitType;
        std::vector<UnitMesh> pieces;
        std::unordered_map<std::string, int> pieceNameToIndices;
        SimVector position;
        SimVector previousPosition;
        std::unique_ptr<CobEnvironment> cobEnvironment;
        PlayerId owner;

        /**
         * Anticlockwise rotation of the unit around the Y axis in radians.
         * The other two axes of rotation are normally determined
         * by the normal of the terrain the unit is standing on.
         */
        SimAngle rotation{0};
        SimAngle previousRotation{0};

        UnitPhysicsInfo physics{UnitPhysicsInfoGround()};

        unsigned int hitPoints{0};

        LifeState lifeState{LifeStateAlive()};

        std::deque<UnitOrder> orders;
        UnitBehaviorState behaviourState;

        /**
         * State we remember related to the current order.
         * This is cleared every time an order is completed.
         * Right now this is just a unit ID for build orders.
         */
        std::optional<UnitId> buildOrderUnitId;

        bool inBuildStance{false};
        bool yardOpen{false};

        /**
         * True if the unit attempted to move last frame
         * and its movement was limited (or prevented entirely) by a collision.
         */
        bool inCollision{false};

        std::array<std::optional<UnitWeapon>, 3> weapons;

        UnitFireOrders fireOrders{UnitFireOrders::FireAtWill};

        unsigned int buildTimeCompleted{0};

        bool activated{false};
        bool isSufficientlyPowered{false};

        Energy energyProductionBuffer{0};
        Metal metalProductionBuffer{0};
        Energy previousEnergyConsumptionBuffer{0};
        Metal previousMetalConsumptionBuffer{0};
        Energy energyConsumptionBuffer{0};
        Metal metalConsumptionBuffer{0};

        std::deque<std::pair<std::string, int>> buildQueue;
        FactoryBehaviorState factoryState;

        static SimAngle toRotation(const SimVector& direction);

        static SimVector toDirection(SimAngle rotation);

        UnitState(const std::vector<UnitMesh>& pieces, std::unique_ptr<CobEnvironment>&& cobEnvironment);

        bool isBeingBuilt(const UnitDefinition& unitDefinition) const;

        unsigned int getBuildPercentLeft(const UnitDefinition& unitDefinition) const;

        float getPreciseCompletePercent(const UnitDefinition& unitDefinition) const;

        struct BuildCostInfo
        {
            unsigned int workerTime;
            Energy energyCost;
            Metal metalCost;
        };

        BuildCostInfo getBuildCostInfo(const UnitDefinition& unitDefinition, unsigned int buildTimeContribution);

        bool addBuildProgress(const UnitDefinition& unitDefinition, unsigned int buildTimeContribution);

        void moveObject(const std::string& pieceName, Axis axis, SimScalar targetPosition, SimScalar speed);

        void moveObjectNow(const std::string& pieceName, Axis axis, SimScalar targetPosition);

        void turnObject(const std::string& pieceName, Axis axis, SimAngle targetAngle, SimScalar speed);

        void turnObjectNow(const std::string& pieceName, Axis axis, SimAngle targetAngle);

        void spinObject(const std::string& pieceName, Axis axis, SimScalar speed, SimScalar acceleration);

        void stopSpinObject(const std::string& pieceName, Axis axis, SimScalar deceleration);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        bool isTurnInProgress(const std::string& pieceName, Axis axis) const;

        bool isOwnedBy(PlayerId playerId) const;

        bool isDead() const;

        void markAsDead();
        void markAsDeadNoCorpse();

        void finishBuilding(const UnitDefinition& unitDefinition);

        void clearOrders();

        void replaceOrders(const std::deque<UnitOrder>& newOrders);

        void addOrder(const UnitOrder& order);

        void setWeaponTarget(unsigned int weaponIndex, UnitId target);
        void setWeaponTarget(unsigned int weaponIndex, const SimVector& target);
        void clearWeaponTarget(unsigned int weaponIndex);
        void clearWeaponTargets();

        Matrix4x<SimScalar> getTransform() const;
        Matrix4x<SimScalar> getInverseTransform() const;

        bool isSelectableBy(const UnitDefinition& unitDefinition, PlayerId player) const;

        void activate();

        void deactivate();

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

        std::optional<std::reference_wrapper<const UnitMesh>> findPiece(const std::string& pieceName) const;

        std::optional<std::reference_wrapper<UnitMesh>> findPiece(const std::string& pieceName);
    };
}
