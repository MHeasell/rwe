#pragma once

#include <boost/functional/hash.hpp>

namespace rwe
{
    struct Point
    {
        int x;
        int y;

        Point() = default;
        Point(int x, int y);

        bool operator==(const Point& rhs) const;

        bool operator!=(const Point& rhs) const;

        Point operator+(const Point& rhs) const;

        Point operator-(const Point& rhs) const;

        int maxSingleDimensionDistance(const Point& rhs) const;
    };
}

namespace std
{
    template <>
    struct hash<rwe::Point>
    {
        std::size_t operator()(const rwe::Point& v) const noexcept
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, v.x);
            boost::hash_combine(seed, v.y);
            return seed;
        }
    };
}

namespace rwe
{
    std::size_t hash_value(const Point& p);
}
