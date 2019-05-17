#ifndef RWE_PATHFINDINGSERVICE_H
#define RWE_PATHFINDINGSERVICE_H

#include <deque>
#include <rwe/GameSimulation.h>
#include <rwe/MovementClassCollisionService.h>
#include <rwe/Point.h>
#include <rwe/UnitId.h>
#include <rwe/math/Vector3f.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/OctileDistance.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/pathfinding/UnitPath.h>

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

            UnitPath operator()(const Vector3f& pos) const
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
        UnitPath findPath(UnitId unitId, const Vector3f& destination);
        UnitPath findPath(UnitId unitId, const DiscreteRect& destination);

        Vector3f getWorldCenter(const DiscreteRect& discreteRect);

        DiscreteRect expandTopLeft(const DiscreteRect& rect, unsigned int width, unsigned int height);
    };
}

#endif
