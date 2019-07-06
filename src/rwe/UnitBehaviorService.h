#ifndef RWE_UNITBEHAVIORSERVICE_H
#define RWE_UNITBEHAVIORSERVICE_H

#include <rwe/UnitFactory.h>
#include <rwe/UnitId.h>
#include <rwe/UnitOrder.h>
#include <rwe/math/Vector3f.h>
#include <rwe/pathfinding/PathFindingService.h>

namespace rwe
{
    class GameScene;

    class UnitBehaviorService
    {
    private:
        GameScene* scene;
        PathFindingService* pathFindingService;
        MovementClassCollisionService* collisionService;
        UnitFactory* unitFactory;

    public:
        UnitBehaviorService(
            GameScene* scene,
            PathFindingService* pathFindingService,
            MovementClassCollisionService* collisionService,
            UnitFactory* unitFactory);

        void update(UnitId unitId);

        // FIXME: shouldn't really be public
        Vector3f getSweetSpot(UnitId id);
        std::optional<Vector3f> tryGetSweetSpot(UnitId id);

    private:
        static std::pair<float, float> computeHeadingAndPitch(float rotation, const Vector3f& from, const Vector3f& to);

        /** Returns true if the order has been completed. */
        bool handleOrder(UnitId unitId, const UnitOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleMoveOrder(UnitId unitId, const MoveOrder& moveOrder);

        /** Returns true if the order has been completed. */
        bool handleAttackOrder(UnitId unitId, const AttackOrder& attackOrder);

        /** Returns true if the order has been completed. */
        bool handleRepairOrder(UnitId unitId, const RepairOrder &buildOrder);

        /** Returns true if the order has been completed. */
        bool handleBuildOrder(UnitId unitId, const BuildOrder& buildOrder);

        /** Returns true if the order has been completed. */
        bool handleBuggerOffOrder(UnitId unitId, const BuggerOffOrder& buggerOffOrder);

        bool handleBuild(UnitId unitId, const std::string& unitType);

        void clearBuild(UnitId unitId);

        bool followPath(Unit& unit, PathFollowingInfo& path);

        void updateWeapon(UnitId id, unsigned int weaponIndex);
        void tryFireWeapon(UnitId id, unsigned int weaponIndex, const Vector3f& targetPosition);

        void applyUnitSteering(UnitId id);
        void updateUnitRotation(UnitId id);
        void updateUnitSpeed(UnitId id);

        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition);

        std::string getAimScriptName(unsigned int weaponIndex) const;
        std::string getAimFromScriptName(unsigned int weaponIndex) const;
        std::string getFireScriptName(unsigned int weaponIndex) const;
        std::string getQueryScriptName(unsigned int weaponIndex) const;

        std::optional<int> runCobQuery(UnitId id, const std::string& name);

        Vector3f getAimingPoint(UnitId id, unsigned int weaponIndex);

        Vector3f getFiringPoint(UnitId id, unsigned int weaponIndex);

        Vector3f getNanoPoint(UnitId id);

        Vector3f getPiecePosition(UnitId id, unsigned int pieceId);

        float getPieceXZRotation(UnitId id, unsigned int pieceId);

        struct BuildPieceInfo
        {
            Vector3f position;
            float rotation;
        };

        BuildPieceInfo getBuildPieceInfo(UnitId id);
    };
}

#endif
