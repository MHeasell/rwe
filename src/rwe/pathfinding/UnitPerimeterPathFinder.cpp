#include "UnitPerimeterPathFinder.h"

namespace rwe
{
    UnitPerimeterPathFinder::UnitPerimeterPathFinder(
        GameSimulation* simulation,
        MovementClassCollisionService* collisionService,
        const UnitId& self,
        const boost::optional<MovementClassId>& movementClass,
        unsigned int footprintX,
        unsigned int footprintZ,
        const DiscreteRect& goalRect)
        : AbstractUnitPathFinder(simulation,
              collisionService,
              self,
              movementClass,
              footprintX,
              footprintZ),
          goalRect(goalRect)
    {
    }

    bool UnitPerimeterPathFinder::isGoal(const Point& vertex)
    {
        return goalRect.isAdjacentTo(vertex.x, vertex.y);
    }

    PathCost UnitPerimeterPathFinder::estimateCostToGoal(const Point& start)
    {
        auto distance = goalRect.octileDistanceToPerimeter(start.x, start.y);
        unsigned int turns = (distance.straight > 0 && distance.diagonal > 0) ? 1 : 0;
        return PathCost(distance, turns);
    }
}
