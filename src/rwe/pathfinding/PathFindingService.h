#pragma once

#include <deque>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/grid/Point.h>
#include <rwe/math/Vector3f.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/OctileDistance.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/UnitPath.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    class GameSimulation;

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
