#pragma once

#include <rwe/grid/DiscreteRect.h>
#include <rwe/grid/Point.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/UnitPath.h>
#include <rwe/sim/MovementClassCollisionService.h>
#include <rwe/sim/SimVector.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    struct GameSimulation;

    class PathFindingService
    {
    public:
        AStarPathInfo<Point, PathCost> lastPathDebugInfo;

        void update(GameSimulation& simulation);

    private:
        UnitPath findPath(const GameSimulation& simulation, UnitId unitId, const SimVector& destination);
        UnitPath findPath(const GameSimulation& simulation, UnitId unitId, const DiscreteRect& destination);

        SimVector getWorldCenter(const GameSimulation& simulation, const DiscreteRect& discreteRect);

        DiscreteRect expandTopLeft(const DiscreteRect& rect, unsigned int width, unsigned int height);
    };
}
