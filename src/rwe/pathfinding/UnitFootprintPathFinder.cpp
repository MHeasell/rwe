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

        NotCollidesPredicate(GameSimulation* simulation, const UnitId& self) : simulation(simulation), self(self)
        {
        }

        bool operator()(const AStarVertexInfo<DiscreteRect>& info)
        {
            return !simulation->isCollisionAt(info.vertex, self);
        }
    };

    UnitFootprintPathFinder::UnitFootprintPathFinder(GameSimulation* simulation, const UnitId& self, const DiscreteRect& goal)
        : simulation(simulation), self(self), goal(goal)
    {
    }

    bool UnitFootprintPathFinder::isGoal(const DiscreteRect& vertex)
    {
        return vertex == goal;
    }

    float UnitFootprintPathFinder::estimateCostToGoal(const DiscreteRect& start)
    {
        auto deltaX = std::abs(goal.x - start.x);
        auto deltaY = std::abs(goal.y - start.y);
        auto pair = std::minmax(deltaX, deltaY);
        auto deltaDiff = pair.second - pair.first;
        return (pair.first * DiagonalCost) + (deltaDiff * StraightCost);
    }

    std::vector<UnitFootprintPathFinder::VertexInfo>
    UnitFootprintPathFinder::getSuccessors(const VertexInfo& info)
    {
        // clang-format off
        std::vector<AStarPathFinder<DiscreteRect>::VertexInfo> v{
            VertexInfo{info.costToReach + StraightCost, DiscreteRect(info.vertex.x + 1, info.vertex.y    , info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + DiagonalCost, DiscreteRect(info.vertex.x + 1, info.vertex.y + 1, info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + StraightCost, DiscreteRect(info.vertex.x    , info.vertex.y + 1, info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + DiagonalCost, DiscreteRect(info.vertex.x - 1, info.vertex.y + 1, info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + StraightCost, DiscreteRect(info.vertex.x - 1, info.vertex.y    , info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + DiagonalCost, DiscreteRect(info.vertex.x - 1, info.vertex.y - 1, info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + StraightCost, DiscreteRect(info.vertex.x    , info.vertex.y - 1, info.vertex.width, info.vertex.height), &info},
            VertexInfo{info.costToReach + DiagonalCost, DiscreteRect(info.vertex.x + 1, info.vertex.y - 1, info.vertex.width, info.vertex.height), &info},
        };
        // clang-format on

        NotCollidesPredicate pred(simulation, self);

        auto a = boost::make_filter_iterator(pred, v.begin(), v.end());
        auto b = boost::make_filter_iterator(pred, v.end(), v.end());

        std::vector<AStarPathFinder<DiscreteRect>::VertexInfo> w;

        std::copy(a, b, std::back_inserter(w));

        return w;
    }
}
