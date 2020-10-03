#pragma once

#include <deque>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/Point.h>
#include <rwe/math/Vector3f.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/OctileDistance.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/UnitPath.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    class PathFindingService
    {
    private:
        class FindPathVisitor
        {
        private:
            PathFindingService* svc;
            UnitId unitId;

        public:
            FindPathVisitor(PathFindingService* svc, const UnitId& unitId) : svc(svc), unitId(unitId)
            {
            }

            UnitPath operator()(const SimVector& pos) const
            {
                return svc->findPath(unitId, pos);
            }
            UnitPath operator()(const DiscreteRect& pos) const
            {
                return svc->findPath(unitId, pos);
            }
        };

        GameSimulation* const simulation;
        MovementClassCollisionService* const collisionService;

    public:
        PathFindingService(GameSimulation* simulation, MovementClassCollisionService* collisionService);

        AStarPathInfo<Point, PathCost> lastPathDebugInfo;

        void update();

    private:
        UnitPath findPath(UnitId unitId, const SimVector& destination);
        UnitPath findPath(UnitId unitId, const DiscreteRect& destination);

        SimVector getWorldCenter(const DiscreteRect& discreteRect);

        DiscreteRect expandTopLeft(const DiscreteRect& rect, unsigned int width, unsigned int height);
    };
}
