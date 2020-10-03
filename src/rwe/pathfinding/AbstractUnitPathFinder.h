#pragma once

#include <rwe/MovementClassCollisionService.h>
#include <rwe/grid/DiscreteRect.h>
#include <rwe/grid/EightWayDirection.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/pathfinding_utils.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    /**
     * Standard unit pathfinder.
     */
    class AbstractUnitPathFinder : public AStarPathFinder<Point, PathCost>
    {
    private:
        GameSimulation* const simulation;
        MovementClassCollisionService* const collisionService;
        const UnitId self;
        const std::optional<MovementClassId> movementClass;
        const unsigned int footprintX;
        const unsigned int footprintZ;

    public:
        AbstractUnitPathFinder(
            GameSimulation* simulation,
            MovementClassCollisionService* collisionService,
            UnitId self,
            std::optional<MovementClassId> movementClass,
            unsigned int footprintX,
            unsigned int footprintZ);

    protected:
        std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) override;

    private:
        bool isWalkable(const Point& p) const;

        bool isWalkable(int x, int y) const;

        bool isRoughTerrain(const Point& p) const;

        Point step(const Point& p, Direction d) const;

        std::vector<Point> getNeighbours(const Point& p);
    };
}
