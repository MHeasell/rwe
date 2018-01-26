#ifndef RWE_PATHFINDINGSERVICE_H
#define RWE_PATHFINDINGSERVICE_H

#include "UnitPath.h"
#include "rwe/GameSimulation.h"
#include "rwe/UnitId.h"
#include <deque>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    class PathFindingService
    {
    private:
        GameSimulation* const simulation;
        MovementClassCollisionService* const collisionService;

    public:
        PathFindingService(GameSimulation* simulation, MovementClassCollisionService* collisionService);

        void update();

    private:
        UnitPath findPath(UnitId unitId, const Vector3f& destination);

        Vector3f getWorldCenter(const DiscreteRect& discreteRect);
    };
}

#endif
