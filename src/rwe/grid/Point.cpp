#include "Point.h"

namespace rwe
{
    Point::Point(int x, int y) : x(x), y(y)
    {
    }

    bool Point::operator==(const Point& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool Point::operator!=(const Point& rhs) const
    {
        return !(rhs == *this);
    }

    Point Point::operator+(const Point& rhs) const
    {
        return Point(x + rhs.x, y + rhs.y);
    }

    Point Point::operator-(const Point& rhs) const
    {
        return Point(x - rhs.x, y - rhs.y);
    }

    std::size_t hash_value(const Point& p)
    {
        return std::hash<Point>()(p);
    }

    int Point::maxSingleDimensionDistance(const Point& rhs) const
    {
        auto delta = *this - rhs;
        return std::max(std::abs(delta.x), std::abs(delta.y));
    }
}
