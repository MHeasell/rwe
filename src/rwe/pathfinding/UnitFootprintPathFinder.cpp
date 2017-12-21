#include "UnitFootprintPathFinder.h"

#include <boost/iterator/filter_iterator.hpp>
#include <cmath>

namespace rwe
{
    static const float StraightCost = 1.0f;
    static constexpr float DiagonalCost = std::sqrt(2.0f);

    struct NotCollidesPredicate
    {
        GameSimulation* simulation;
        UnitId self;
        unsigned int footprintX;
        unsigned int footprintZ;

        NotCollidesPredicate(GameSimulation* simulation, const UnitId& self, unsigned int footprintX, unsigned int footprintZ)
            : simulation(simulation), self(self), footprintX(footprintX), footprintZ(footprintZ)
        {
        }

        template <typename Cost>
        bool operator()(const AStarVertexInfo<PathVertex, Cost>& info)
        {
            DiscreteRect rect(info.vertex.position.x, info.vertex.position.y, footprintX, footprintZ);
            return !simulation->isCollisionAt(rect, self);
        }
    };

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
            auto distanceCost = isDiagonal(d) ? DiagonalCost : StraightCost;
            PathCost cost{distanceCost, directionDistance(info.vertex.direction, d)};

            auto newPosition = pos + directionToPoint(d);
            v.push_back(VertexInfo{info.costToReach + cost, PathVertex(newPosition, d), &info});
        }

        NotCollidesPredicate pred(simulation, self, footprintX, footprintZ);

        auto a = boost::make_filter_iterator(pred, v.begin(), v.end());
        auto b = boost::make_filter_iterator(pred, v.end(), v.end());

        std::vector<VertexInfo> w;
        v.reserve(v.size());

        std::copy(a, b, std::back_inserter(w));

        return w;
    }
}
