#pragma once

#include <rwe/DiscreteRect.h>
#include <rwe/EightWayDirection.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/AbstractUnitPathFinder.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/pathfinding_utils.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    /**
     * Standard unit pathfinder.
     */
    class UnitPathFinder : public AbstractUnitPathFinder
    {
    private:
        const Point goal;

    public:
        UnitPathFinder(
            GameSimulation* simulation,
            MovementClassCollisionService* collisionService,
            UnitId self,
            std::optional<MovementClassId> movementClass,
            unsigned int footprintX,
            unsigned int footprintZ,
            const Point& goal);

    protected:
        bool isGoal(const Point& vertex) override;

        PathCost estimateCostToGoal(const Point& start) override;
    };
}
