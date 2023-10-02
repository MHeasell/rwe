#pragma once

#include <optional>
#include <rwe/pathfinding/PathFindingService.h>
#include <rwe/sim/GameSimulation.h>
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

        void updateWind(SimScalar windSpeed, SimAngle windDirection);

        void update(UnitId unitId);

        // FIXME: shouldn't really be public
        SimVector getSweetSpot(UnitId id);
        std::optional<SimVector> tryGetSweetSpot(UnitId id);

    private:
        /** Returns true if the order has been completed. */
        bool handleOrder(UnitInfo unitInfo, const UnitOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleMoveOrder(UnitInfo unitInfo, const MoveOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleAttackOrder(UnitInfo unitInfo, const AttackOrder& attackOrder);

        /** Returns true if the order has been completed. */
        bool handleBuildOrder(UnitInfo unitInfo, const BuildOrder& buildOrder);

        /** Returns true if the order has been completed. */
        bool handleBuggerOffOrder(UnitInfo unitInfo, const BuggerOffOrder& buggerOffOrder);

        /** Returns true if the order has been completed. */
        bool handleCompleteBuildOrder(UnitInfo unitInfo, const CompleteBuildOrder& buildOrder);

        bool handleGuardOrder(UnitInfo unitInfo, const GuardOrder& guardOrder);

        bool handleBuild(UnitInfo unitInfo, const std::string& unitType);

        void clearBuild(UnitInfo unitInfo);

        void updateWeapon(UnitId id, unsigned int weaponIndex);

        SimVector changeDirectionByRandomAngle(const SimVector& direction, SimAngle maxAngle);

        void tryFireWeapon(UnitId id, unsigned int weaponIndex);

        SimVector getUnitPositionWithCache(UnitState& s, UnitId unitId);

        void updateNavigation(UnitInfo unitInfo);

        void applyUnitSteering(UnitInfo unitInfo);
        void updateUnitRotation(UnitInfo unitInfo);
        void updateUnitSpeed(UnitInfo unitInfo);

        void updateGroundUnitPosition(UnitInfo unitInfo, const UnitPhysicsInfoGround& physics);
        void updateUnitPosition(UnitInfo unitInfo);

        bool tryApplyMovementToPosition(UnitInfo unitInfo, const SimVector& newPosition);

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

        PathDestination resolvePathDestination(UnitState& s, const MovingStateGoal& goal);

        void groundUnitMoveTo(UnitInfo unitInfo, const MovingStateGoal& goal);

        bool flyingUnitMoveTo(UnitInfo unitInfo, const MovingStateGoal& goal);

        bool navigateTo(UnitInfo unitInfo, const NavigationGoal& goal);

        void moveTo(UnitInfo unitInfo, const MovingStateGoal& goal);

        bool attackTarget(UnitInfo unitInfo, const AttackTarget& target);

        bool buildUnit(UnitInfo unitInfo, const std::string& unitType, const SimVector& position);

        UnitCreationStatus createNewUnit(UnitInfo unitInfo, const std::string& unitType, const SimVector& position);

        bool buildExistingUnit(UnitInfo unitInfo, UnitId targetUnitId);

        void changeState(UnitState& unit, const UnitBehaviorState& newState);

        bool deployBuildArm(UnitInfo unitInfo, UnitId targetUnitId);

        bool climbToCruiseAltitude(UnitInfo unitInfo);

        bool descendToGroundLevel(UnitInfo unitInfo);

        void transitionFromGroundToAir(UnitInfo unitInfo);
        bool tryTransitionFromAirToGround(UnitInfo unitInfo);

        bool flyTowardsGoal(UnitInfo unitInfo, const MovingStateGoal& goal);
    };
}
