#pragma once

#include <rwe/grid/Point.h>
#include <rwe/pathfinding/AbstractUnitPathFinder.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/MovementClassCollisionService.h>
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
            const GameSimulation* simulation,
            const MovementClassCollisionService* collisionService,
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
