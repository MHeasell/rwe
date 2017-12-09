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
        bool tryApplyMovementToPosition(UnitId id, const Vector3f& newPosition);
    };
}

#endif
