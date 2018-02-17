#ifndef RWE_UNITFOOTPRINTPATHFINDER_H
#define RWE_UNITFOOTPRINTPATHFINDER_H

#include "AbstractUnitPathFinder.h"
#include "PathCost.h"
#include "pathfinding_utils.h"
#include <rwe/DiscreteRect.h>
#include <rwe/EightWayDirection.h>
#include <rwe/GameSimulation.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/UnitId.h>
#include <rwe/pathfinding/AStarPathFinder.h>

namespace rwe
{
    /**
     * Standard unit pathfinder.
     */
    class UnitFootprintPathFinder : public AbstractUnitPathFinder
    {
    private:
        const Point goal;

    public:
        UnitFootprintPathFinder(
            GameSimulation* simulation,
            MovementClassCollisionService* collisionService,
            UnitId self,
            boost::optional<MovementClassId> movementClass,
            unsigned int footprintX,
            unsigned int footprintZ,
            const Point& goal);

    protected:
        bool isGoal(const Point& vertex) override;

        PathCost estimateCostToGoal(const Point& start) override;
    };
}

#endif
