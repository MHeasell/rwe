#include "UnitFootprintPathFinder.h"

namespace rwe
{
    UnitFootprintPathFinder::UnitFootprintPathFinder(
        GameSimulation* simulation,
        MovementClassCollisionService* collisionService,
        UnitId self,
        boost::optional<MovementClassId> movementClass,
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

    bool UnitFootprintPathFinder::isGoal(const Point& vertex)
    {
        return vertex == goal;
    }

    PathCost UnitFootprintPathFinder::estimateCostToGoal(const Point& start)
    {
        auto distance = octileDistance(start, goal);
        unsigned int turns = (distance.straight > 0 && distance.diagonal > 0) ? 1 : 0;
        return PathCost(distance, turns);
    }
}
