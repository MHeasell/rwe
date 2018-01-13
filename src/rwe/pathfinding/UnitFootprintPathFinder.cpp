#include "UnitFootprintPathFinder.h"

namespace rwe
{
    static const float StraightCost = 1.0f;
    static constexpr float DiagonalCost = std::sqrt(2.0f);

    UnitFootprintPathFinder::UnitFootprintPathFinder(
        GameSimulation* simulation,
        const UnitId& self,
        unsigned int footprintX,
        unsigned int footprintZ,
        const Point& goal)
        : simulation(simulation),
          self(self),
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
        std::vector<Point> neighbours;
        if (info.predecessor)
        {
            auto delta = info.vertex - (*info.predecessor)->vertex;
            assert(delta.x != 0 || delta.y != 0);
            assert(delta.x == 0 || delta.y == 0 || std::abs(delta.x) == std::abs(delta.y));
            auto dx = std::clamp(delta.x, -1, 1);
            auto dy = std::clamp(delta.y, -1, 1);
            neighbours = getNeighbours(info.vertex, pointToDirection(Point(dx, dy)));
        }
        else
        {
            neighbours = getNeighbours(info.vertex);
        }

        std::vector<VertexInfo> vs;
        for (const auto& neighbour : neighbours)
        {
            auto jumpedNeighbour = jump(neighbour, info.vertex);
            if (!jumpedNeighbour)
            {
                continue;
            }

            auto octile = octileDistance(info.vertex, *jumpedNeighbour);
            assert(octile.diagonal == 0 || octile.straight == 0);
            auto cost = (octile.straight * StraightCost) + (octile.diagonal + DiagonalCost);
            vs.push_back(VertexInfo{info.costToReach + cost, *jumpedNeighbour, &info});
        }

        return vs;
    }

    bool UnitFootprintPathFinder::isWalkable(const Point& p) const
    {
        DiscreteRect rect(p.x, p.y, footprintX, footprintZ);
        return !simulation->isCollisionAt(rect, self);
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

    std::vector<Point> UnitFootprintPathFinder::getNeighbours(const Point& p, Direction travelDirection)
    {
        std::vector<Point> neighbours;

        auto dir = directionToPoint(travelDirection);
        auto dx = dir.x;
        auto dy = dir.y;

        if (dx != 0 && dy != 0) // diagonal movement
        {
            // check natural neighbours
            Point up(p.x, p.y + dy);
            if (isWalkable(up))
            {
                neighbours.push_back(up);
            }

            Point right(p.x + dx, p.y);
            if (isWalkable(right))
            {
                neighbours.push_back(right);
            }

            Point upRight(p.x + dx, p.y + dy);
            if (isWalkable(upRight))
            {
                neighbours.push_back(upRight);
            }

            // check forced neighbours
            Point left(p.x - dx, p.y);
            Point upLeft(p.x - dx, p.y + dy);
            if (isWalkable(upLeft) && !isWalkable(left))
            {
                neighbours.push_back(upLeft);
            }

            Point down(p.x, p.y - dy);
            Point downRight(p.x + dx, p.y - dy);
            if (isWalkable(downRight) && !isWalkable(down))
            {
                neighbours.push_back(downRight);
            }
        }
        else if (dx == 0) // vertical movement
        {
            // check natural neighbours
            Point forward(p.x, p.y + dy);
            if (isWalkable(forward))
            {
                neighbours.push_back(forward);
            }

            // check forced neighbours
            Point left(p.x - 1, p.y);
            Point leftForward(p.x - 1, p.y + dy);
            if (isWalkable(leftForward) && !isWalkable(left))
            {
                neighbours.push_back(leftForward);
            }

            Point right(p.x + 1, p.y);
            Point rightForward(p.x + 1, p.y + dy);
            if (isWalkable(rightForward) && !isWalkable(right))
            {
                neighbours.push_back(rightForward);
            }
        }
        else // horizontal movement
        {
            // check natural neighbours
            Point forward(p.x + dx, p.y);
            if (isWalkable(forward))
            {
                neighbours.push_back(forward);
            }

            // check forced neighbours
            Point up(p.x, p.y - 1);
            Point upForward(p.x + dx, p.y - 1);
            if (isWalkable(upForward) && !isWalkable(up))
            {
                neighbours.push_back(upForward);
            }

            Point down(p.x, p.y + 1);
            Point downForward(p.x + dx, p.y + 1);
            if (isWalkable(downForward) && !isWalkable(down))
            {
                neighbours.push_back(downForward);
            }
        }

        return neighbours;
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

    boost::optional<Point> UnitFootprintPathFinder::jump(const Point& p, const Point& parent)
    {
        auto directionVector = p - parent;
        auto dx = directionVector.x;
        auto dy = directionVector.y;

        if (!isWalkable(p))
        {
            return boost::none;
        }

        if (isGoal(p))
        {
            return p;
        }

        if (dx != 0 && dy != 0) // diagonal
        {
            // check for forced neighbours
            if ((isWalkable(p.x - dx, p.y + dy) && !isWalkable(p.x - dx, p.y))
                || (isWalkable(p.x + dx, p.y - dy) && !isWalkable(p.x, p.y - dy)))
            {
                return p;
            }

            // check for vertical or horizontal jump points
            if (jump(Point(p.x + dx, p.y), p) || jump(Point(p.x, p.y + dy), p))
            {
                return p;
            }
        }
        else if (dx != 0) // horizontal
        {
            if ((isWalkable(p.x + dx, p.y + 1) && !isWalkable(p.x, p.y + 1))
                || (isWalkable(p.x + dx, p.y - 1) && !isWalkable(p.x, p.y - 1)))
            {
                return p;
            }
        }
        else // vertical
        {
            if ((isWalkable(p.x - 1, p.y + dy) && !isWalkable(p.x - 1, p.y))
                || (isWalkable(p.x + 1, p.y + dy) && !isWalkable(p.x + 1, p.y)))
            {
                return p;
            }
        }

        return jump(p + directionVector, p);
    }
}
