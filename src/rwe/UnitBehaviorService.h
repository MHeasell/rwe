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
        void updateWeapon(UnitId id, unsigned int weaponIndex);
        void tryFireWeapon(UnitId id, unsigned int weaponIndex);

        void applyUnitSteering(UnitId id, float targetAngle, float targetSpeed);
        void updateUnitRotation(UnitId id, float targetAngle);
        void updateUnitSpeed(UnitId id, float targetSpeed);

        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition);

        std::string getAimScriptName(unsigned int weaponIndex) const;
    };
}

#endif
