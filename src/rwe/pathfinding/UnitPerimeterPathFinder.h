#pragma once

#include <rwe/grid/DiscreteRect.h>
#include <rwe/pathfinding/AbstractUnitPathFinder.h>

namespace rwe
{
    /**
     * Unit pathfinder where the goal is any cell
     * that is adjacent to a rectagular region.
     */
    class UnitPerimeterPathFinder : public AbstractUnitPathFinder
    {
    private:
        const DiscreteRect goalRect;

    protected:
    public:
        UnitPerimeterPathFinder(
            GameSimulation* simulation,
            MovementClassCollisionService* collisionService,
            const UnitId& self,
            const std::optional<MovementClassId>& movementClass,
            int footprintX,
            int footprintZ,
            const DiscreteRect& goalRect);

    protected:
        bool isGoal(const Point& vertex) override;

        PathCost estimateCostToGoal(const Point& start) override;
    };
}
