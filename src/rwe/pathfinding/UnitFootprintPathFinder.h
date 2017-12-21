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
    class UnitFootprintPathFinder : public AStarPathFinder<PathVertex, PathCost>
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
        bool isGoal(const PathVertex& vertex) override;

        PathCost estimateCostToGoal(const PathVertex& start) override;

        std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) override;
    };
}

#endif
