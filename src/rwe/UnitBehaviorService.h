#ifndef RWE_UNITBEHAVIORSERVICE_H
#define RWE_UNITBEHAVIORSERVICE_H

#include "UnitId.h"
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

    public:
        UnitBehaviorService(GameScene* scene, PathFindingService* pathFindingService, MovementClassCollisionService* collisionService);

        void update(UnitId unitId);

    private:
        bool followPath(Unit& unit, PathFollowingInfo& path);

        void updateWeapon(UnitId id, unsigned int weaponIndex);
        void tryFireWeapon(UnitId id, unsigned int weaponIndex);

        void applyUnitSteering(UnitId id);
        void updateUnitRotation(UnitId id);
        void updateUnitSpeed(UnitId id);

        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition);

        std::string getAimScriptName(unsigned int weaponIndex) const;
    };
}

#endif
