#include "UnitPathFinder.h"

namespace rwe
{
    UnitPathFinder::UnitPathFinder(
        GameSimulation* simulation,
        MovementClassCollisionService* collisionService,
        UnitId self,
        std::optional<MovementClassId> movementClass,
        unsigned int footprintX,
        unsigned int footprintZ,
        const Point& goal)
        : AbstractUnitPathFinder(
            simulation,
            collisionService,
            self,
            movementClass,
            footprintX,
            footprintZ),
          goal(goal)
    {
    }

    bool UnitPathFinder::isGoal(const Point& vertex)
    {
        return vertex == goal;
    }

    PathCost UnitPathFinder::estimateCostToGoal(const Point& start)
    {
        auto distance = octileDistance(start, goal);
        unsigned int turns = (distance.straight > 0 && distance.diagonal > 0) ? 1 : 0;
        return PathCost(distance, turns);
    }
}
