#include "UnitFootprintPathFinder.h"

namespace rwe
{
    UnitFootprintPathFinder::UnitFootprintPathFinder(
        GameSimulation* simulation,
        MovementClassCollisionService* collisionService,
        UnitId self,
        boost::optional<MovementClassId> movementClass,
        unsigned int footprintX,
        unsigned int footprintZ,
        const Point& goal)
        : simulation(simulation),
          collisionService(collisionService),
          self(self),
          movementClass(movementClass),
          footprintX(footprintX),
          footprintZ(footprintZ),
          goal(goal)
    {
    }

    bool UnitFootprintPathFinder::isGoal(const Point& vertex)
    {
        return vertex == goal;
    }

    PathCost UnitFootprintPathFinder::estimateCostToGoal(const Point& start)
    {
        auto distance = octileDistance(start, goal);
        unsigned int turns = (distance.straight > 0 && distance.diagonal > 0) ? 1 : 0;
        return PathCost(distance, turns);
    }

    std::vector<UnitFootprintPathFinder::VertexInfo>
    UnitFootprintPathFinder::getSuccessors(const VertexInfo& info)
    {
        boost::optional<Direction> prevDirection;
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
            unsigned int turns = (!prevDirection || direction == *prevDirection) ? 0 : 1;
            PathCost cost(distance, turns);
            vs.push_back(VertexInfo{info.costToReach + cost, neighbour, &info});
        }

        return vs;
    }

    bool UnitFootprintPathFinder::isWalkable(const Point& p) const
    {
        DiscreteRect rect(p.x, p.y, footprintX, footprintZ);
        return (movementClass ? collisionService->isWalkable(*movementClass, p) : true) && !simulation->isCollisionAt(rect, self);
    }

    bool UnitFootprintPathFinder::isWalkable(int x, int y) const
    {
        return isWalkable(Point(x, y));
    }

    bool UnitFootprintPathFinder::isRoughTerrain(const Point& p) const
    {
        DiscreteRect rect(p.x, p.y, footprintX, footprintZ);
        return simulation->isAdjacentToObstacle(rect, self);
    }

    Point UnitFootprintPathFinder::step(const Point& p, Direction d) const
    {
        auto directionVector = directionToPoint(d);
        return p + directionVector;
    }

    std::vector<Point> UnitFootprintPathFinder::getNeighbours(const Point& p)
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
