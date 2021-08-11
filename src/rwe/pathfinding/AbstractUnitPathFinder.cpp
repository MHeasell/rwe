#include "AbstractUnitPathFinder.h"

namespace rwe
{
    AbstractUnitPathFinder::AbstractUnitPathFinder(
        GameSimulation* simulation,
        MovementClassCollisionService* collisionService,
        UnitId self,
        std::optional<MovementClassId> movementClass,
        int footprintX,
        int footprintZ)
        : simulation(simulation),
          collisionService(collisionService),
          self(self),
          movementClass(movementClass),
          footprintX(footprintX),
          footprintZ(footprintZ)
    {
    }

    std::vector<AbstractUnitPathFinder::VertexInfo>
    AbstractUnitPathFinder::getSuccessors(const VertexInfo& info)
    {
        std::optional<Direction> prevDirection;
        if (info.predecessor)
        {
            prevDirection = pointToDirection(info.vertex - (*info.predecessor)->vertex);
        }

        auto neighbours = getNeighbours(info.vertex);

        std::vector<VertexInfo> vs;
        for (const auto& neighbour : neighbours)
        {
            auto direction = pointToDirection(neighbour - info.vertex);
            auto distance = octileDistance(info.vertex, neighbour);
            assert(distance.diagonal == 0 || distance.straight == 0);
            if (isRoughTerrain(neighbour))
            {
                // double the cost on rough terrain
                distance = distance + distance;
            }
            int turns = (!prevDirection || direction == *prevDirection) ? 0 : 1;
            PathCost cost(distance, turns);
            vs.push_back(VertexInfo{info.costToReach + cost, neighbour, &info});
        }

        return vs;
    }

    bool AbstractUnitPathFinder::isWalkable(const Point& p) const
    {
        DiscreteRect rect(p.x, p.y, footprintX, footprintZ);
        return (movementClass ? collisionService->isWalkable(*movementClass, p) : true) && !simulation->isCollisionAt(rect, self);
    }

    bool AbstractUnitPathFinder::isWalkable(int x, int y) const
    {
        return isWalkable(Point(x, y));
    }

    bool AbstractUnitPathFinder::isRoughTerrain(const Point& p) const
    {
        DiscreteRect rect(p.x, p.y, footprintX, footprintZ);
        return simulation->isAdjacentToObstacle(rect);
    }

    Point AbstractUnitPathFinder::step(const Point& p, Direction d) const
    {
        auto directionVector = directionToPoint(d);
        return p + directionVector;
    }

    std::vector<Point> AbstractUnitPathFinder::getNeighbours(const Point& p)
    {
        std::vector<Point> neighbours;
        neighbours.reserve(Directions.size());

        for (auto d : Directions)
        {
            auto newPosition = step(p, d);

            if (!isWalkable(newPosition))
            {
                continue;
            }

            neighbours.push_back(newPosition);
        }

        return neighbours;
    }
}
