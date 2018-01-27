#include "UnitFootprintPathFinder.h"

namespace rwe
{
    static const float StraightCost = 1.0f;
    static constexpr float DiagonalCost = std::sqrt(2.0f);

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

    float UnitFootprintPathFinder::estimateCostToGoal(const Point& start)
    {
        auto octile = octileDistance(start, goal);
        return (octile.diagonal * DiagonalCost) + (octile.straight * StraightCost);
    }

    std::vector<UnitFootprintPathFinder::VertexInfo>
    UnitFootprintPathFinder::getSuccessors(const VertexInfo& info)
    {
        auto neighbours = getNeighbours(info.vertex);

        std::vector<VertexInfo> vs;
        for (const auto& neighbour : neighbours)
        {
            auto octile = octileDistance(info.vertex, neighbour);
            assert(octile.diagonal == 0 || octile.straight == 0);
            auto cost = (octile.straight * StraightCost) + (octile.diagonal + DiagonalCost);
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
