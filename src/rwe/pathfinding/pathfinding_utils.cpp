#include "pathfinding_utils.h"

namespace rwe
{
    std::vector<Point> runSimplifyPath(const std::vector <Point>& input)
    {
        std::vector<Point> output(input.size());
        auto count = simplifyPath(input.begin(), input.end(), output.begin());
        output.resize(count);
        return output;
    }

    std::ostream& operator<<(std::ostream& stream, const OctileDistance& d)
    {
        stream << "(s:" << d.straight << ", " << d.diagonal << ")";
        return stream;
    }

    OctileDistance octileDistance(const Point& start, const Point& goal)
    {
        auto deltaX = static_cast<unsigned int>(std::abs(goal.x - start.x));
        auto deltaY = static_cast<unsigned int>(std::abs(goal.y - start.y));
        auto pair = std::minmax(deltaX, deltaY);
        auto deltaDiff = pair.second - pair.first;
        return OctileDistance{deltaDiff, pair.first};
    }
}
