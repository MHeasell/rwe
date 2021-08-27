#include "pathfinding_utils.h"
#include <algorithm>

namespace rwe
{
    std::vector<Point> runSimplifyPath(const std::vector<Point>& input)
    {
        std::vector<Point> output(input.size());
        auto count = simplifyPath(input.begin(), input.end(), output.begin());
        output.resize(count);
        return output;
    }

    OctileDistance octileDistance(const Point& start, const Point& goal)
    {
        int deltaX = std::abs(goal.x - start.x);
        int deltaY = std::abs(goal.y - start.y);
        auto pair = std::minmax(deltaX, deltaY);
        auto deltaDiff = pair.second - pair.first;
        return OctileDistance{deltaDiff, pair.first};
    }
}
