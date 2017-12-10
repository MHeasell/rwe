#ifndef RWE_UNITFOOTPRINTPATHFINDER_H
#define RWE_UNITFOOTPRINTPATHFINDER_H

#include <rwe/DiscreteRect.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/GameSimulation.h>
#include <rwe/UnitId.h>

namespace rwe
{
    class UnitFootprintPathFinder : public AStarPathFinder<DiscreteRect>
    {
    private:
        GameSimulation* simulation;
        UnitId self;

    public:
        UnitFootprintPathFinder(GameSimulation* simulation, const UnitId& self);

    protected:
        float estimateCost(const DiscreteRect& start, const DiscreteRect& goal) override;

        std::vector<VertexInfo> getSuccessors(const VertexInfo& vertex) override;
    };
}

#endif
