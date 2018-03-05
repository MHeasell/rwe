#ifndef RWE_UNITPERIMETERPATHFINDER_H
#define RWE_UNITPERIMETERPATHFINDER_H

#include <rwe/DiscreteRect.h>
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
            const boost::optional<MovementClassId>& movementClass,
            unsigned int footprintX,
            unsigned int footprintZ,
            const DiscreteRect& goalRect);

    protected:
        bool isGoal(const Point& vertex) override;

        PathCost estimateCostToGoal(const Point& start) override;
    };
}

#endif
