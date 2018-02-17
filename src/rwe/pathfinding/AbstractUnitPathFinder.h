#ifndef RWE_ABSTRACTUNITPATHFINDER_H
#define RWE_ABSTRACTUNITPATHFINDER_H

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
    class AbstractUnitPathFinder : public AStarPathFinder<Point, PathCost>
    {
    private:
        GameSimulation* const simulation;
        MovementClassCollisionService* const collisionService;
        const UnitId self;
        const boost::optional<MovementClassId> movementClass;
        const unsigned int footprintX;
        const unsigned int footprintZ;

    public:
        AbstractUnitPathFinder(
            GameSimulation* simulation,
            MovementClassCollisionService* collisionService,
            UnitId self,
            boost::optional<MovementClassId> movementClass,
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

#endif
