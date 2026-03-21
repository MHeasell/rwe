#pragma once

#include <functional>
#include <rwe/util/hash_combine.h>

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

        Point& operator+=(const Point& rhs);

        Point& operator-=(const Point& rhs);


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
            rwe::hashCombine(seed, std::hash<int>{}(v.x));
            rwe::hashCombine(seed, std::hash<int>{}(v.y));
            return seed;
        }
    };
}
