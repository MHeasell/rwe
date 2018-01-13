#ifndef RWE_UNITFOOTPRINTPATHFINDER_H
#define RWE_UNITFOOTPRINTPATHFINDER_H

#include <rwe/DiscreteRect.h>
#include <rwe/EightWayDirection.h>
#include <rwe/GameSimulation.h>
#include <rwe/UnitId.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include "pathfinding_utils.h"

namespace rwe
{
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

        bool isNearObstacle(const Point& p) const;

        float costToGo(const Point& p, Direction d) const;

        Point step(const Point& p, Direction d) const;

        std::vector<Point> getNeighbours(const Point& p);

        std::vector<Point> getNeighbours(const Point& p, Direction travelDirection);
    };
}

#endif
