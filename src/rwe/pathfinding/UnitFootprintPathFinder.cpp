#include "UnitFootprintPathFinder.h"

#include <boost/iterator/filter_iterator.hpp>
#include <cmath>

namespace rwe
{
    static const float StraightCost = 1.0f;
    static constexpr float DiagonalCost = std::sqrt(2.0f);

    UnitFootprintPathFinder::UnitFootprintPathFinder(GameSimulation* simulation, const UnitId& self, unsigned int footprintX, unsigned int footprintZ, const Point& goal)
        : simulation(simulation), self(self), footprintX(footprintX), footprintZ(footprintZ), goal(goal)
    {
    }

    bool UnitFootprintPathFinder::isGoal(const PathVertex& vertex)
    {
        return vertex.position == goal;
    }

    PathCost UnitFootprintPathFinder::estimateCostToGoal(const PathVertex& start)
    {
        auto deltaX = std::abs(goal.x - start.position.x);
        auto deltaY = std::abs(goal.y - start.position.y);
        auto pair = std::minmax(deltaX, deltaY);
        auto deltaDiff = pair.second - pair.first;
        auto distanceCost = (pair.first * DiagonalCost) + (deltaDiff * StraightCost);

        return PathCost{distanceCost, 0};
    }

    std::vector<UnitFootprintPathFinder::VertexInfo>
    UnitFootprintPathFinder::getSuccessors(const VertexInfo& info)
    {
        const auto& pos = info.vertex.position;

        std::vector<VertexInfo> v;
        v.reserve(Directions.size());

        for (auto d : Directions)
        {
            auto newPosition = pos + directionToPoint(d);
            DiscreteRect rect(newPosition.x, newPosition.y, footprintX, footprintZ);
            if (simulation->isCollisionAt(rect, self))
            {
                continue;
            }

            auto distanceCost = isDiagonal(d) ? DiagonalCost : StraightCost;

            if (simulation->isAdjacentToObstacle(rect, self))
            {
                distanceCost *= 2.0f;
            }

            PathCost cost{distanceCost, directionDistance(info.vertex.direction, d)};

            v.push_back(VertexInfo{info.costToReach + cost, PathVertex(newPosition, d), &info});
        }

        return v;
    }
}
