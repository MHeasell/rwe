#ifndef RWE_UNITFOOTPRINTPATHFINDER_H
#define RWE_UNITFOOTPRINTPATHFINDER_H

#include "pathfinding_utils.h"
#include <rwe/DiscreteRect.h>
#include <rwe/EightWayDirection.h>
#include <rwe/GameSimulation.h>
#include <rwe/UnitId.h>
#include <rwe/pathfinding/AStarPathFinder.h>

namespace rwe
{
    /**
     * Standard unit pathfinder, implementing jump point search.
     */
    class UnitFootprintPathFinder : public AStarPathFinder<Point, float>
    {
    private:
        GameSimulation* simulation;
        UnitId self;
        unsigned int footprintX;
        unsigned int footprintZ;
        Point goal;

    public:
        UnitFootprintPathFinder(GameSimulation* simulation, const UnitId& self, unsigned int footprintX, unsigned int footprintZ, const Point& goal);

    protected:
        bool isGoal(const Point& vertex) override;

        float estimateCostToGoal(const Point& start) override;

        std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) override;

    private:
        boost::optional<Point> jump(const Point& p, const Point& parent);

        bool isWalkable(const Point& p) const;

        bool isWalkable(int x, int y) const;

        Point step(const Point& p, Direction d) const;

        std::vector<Point> getNeighbours(const Point& p);

        std::vector<Point> getNeighbours(const Point& p, Direction travelDirection);
    };
}

#endif
