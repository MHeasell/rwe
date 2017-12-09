#ifndef RWE_UNITBEHAVIORSERVICE_H
#define RWE_UNITBEHAVIORSERVICE_H

#include <rwe/math/Vector3f.h>
#include "UnitId.h"

namespace rwe
{
    class GameScene;

    class UnitBehaviorService
    {
    private:
        GameScene* scene;

    public:
        explicit UnitBehaviorService(GameScene* scene);

        void update(UnitId unitId);

    private:
        void applyUnitSteering(UnitId id, float targetAngle, float targetSpeed);
        void updateUnitRotation(UnitId id, float targetAngle);
        void updateUnitSpeed(UnitId id, float targetSpeed);

        void updateUnitPosition(UnitId unitId);

        bool tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition);
    };
}

#endif
