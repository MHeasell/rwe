#pragma once

#include <optional>
#include <rwe/pathfinding/PathFindingService.h>
#include <rwe/sim/ProjectilePhysicsType.h>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitId.h>
#include <rwe/sim/UnitOrder.h>
#include <rwe/sim/UnitState.h>
#include <rwe/sim/UnitWeapon.h>
#include <string>
#include <utility>

namespace rwe
{
    class GameScene;

    class UnitBehaviorService
    {
    private:
        GameSimulation* const sim;

    public:
        UnitBehaviorService(GameSimulation* sim);

        void onCreate(UnitId unitId);

        void update(UnitId unitId);

        // FIXME: shouldn't really be public
        SimVector getSweetSpot(UnitId id);
        std::optional<SimVector> tryGetSweetSpot(UnitId id);

    private:
        /** Returns true if the order has been completed. */
        bool handleOrder(UnitId unitId, const UnitOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleAttackOrder(UnitId unitId, const AttackOrder& attackOrder);

        /** Returns true if the order has been completed. */
        bool handleBuildOrder(UnitId unitId, const BuildOrder& buildOrder);

        /** Returns true if the order has been completed. */
        bool handleBuggerOffOrder(UnitId unitId, const BuggerOffOrder& buggerOffOrder);

        /** Returns true if the order has been completed. */
        bool handleCompleteBuildOrder(UnitId unitId, const CompleteBuildOrder& buildOrder);

        bool handleGuardOrder(UnitId unitId, const GuardOrder& guardOrder);

        bool handleBuild(UnitId unitId, const std::string& unitType);

        void clearBuild(UnitId unitId);

        void updateWeapon(UnitId id, unsigned int weaponIndex);

        SimVector changeDirectionByRandomAngle(const SimVector& direction, SimAngle maxAngle);

        void tryFireWeapon(UnitId id, unsigned int weaponIndex);

        void updateNavigation(UnitId id);

        void applyUnitSteering(UnitId id);
        void updateUnitRotation(UnitId id);
        void updateUnitSpeed(UnitId id);

        void updateGroundUnitPosition(UnitId unitId, UnitState& unit, const UnitDefinition& unitDefinition, const UnitPhysicsInfoGround& physics);
        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const SimVector& newPosition);

        std::optional<int> runCobQuery(UnitId id, const std::string& name);

        SimVector getAimingPoint(UnitId id, unsigned int weaponIndex);
        SimVector getLocalAimingPoint(UnitId id, unsigned int weaponIndex);

        SimVector getLocalFiringPoint(UnitId id, unsigned int weaponIndex);

        SimVector getNanoPoint(UnitId id);

        SimVector getPieceLocalPosition(UnitId id, unsigned int pieceId);
        SimVector getPiecePosition(UnitId id, unsigned int pieceId);

        SimAngle getPieceXZRotation(UnitId id, unsigned int pieceId);

        struct BuildPieceInfo
        {
            SimVector position;
            SimAngle rotation;
        };

        BuildPieceInfo getBuildPieceInfo(UnitId id);

        std::optional<SimVector> getTargetPosition(const UnitWeaponAttackTarget& target);

        bool groundUnitMoveTo(UnitId unitId, const MovingStateGoal& goal);

        bool flyingUnitMoveTo(UnitId unitId, const MovingStateGoal& goal);

        bool navigateTo(UnitId unitId, const NavigationGoal& goal);

        bool moveTo(UnitId unitId, const MovingStateGoal& goal);

        bool attackTarget(UnitId unitId, const AttackTarget& target);

        bool buildUnit(UnitId unitId, const std::string& unitType, const SimVector& position);

        UnitCreationStatus createNewUnit(UnitId unitId, const std::string& unitType, const SimVector& position);

        bool buildExistingUnit(UnitId unitId, UnitId targetUnitId);

        void changeState(UnitState& unit, const UnitBehaviorState& newState);

        bool deployBuildArm(UnitId unitId, UnitId targetUnitId);

        bool climbToCruiseAltitude(UnitId unitId);

        bool descendToGroundLevel(UnitId unitId);

        void transitionFromGroundToAir(UnitId unitId);
        bool tryTransitionFromAirToGround(UnitId unitId);

        bool flyTowardsGoal(UnitId unitId, const MovingStateGoal& goal);
    };
}
