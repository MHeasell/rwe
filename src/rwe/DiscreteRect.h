#ifndef RWE_DISCRETERECT_H
#define RWE_DISCRETERECT_H

#include <boost/functional/hash.hpp>
#include <rwe/Point.h>
#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    struct DiscreteRect
    {
        int x;
        int y;
        unsigned int width;
        unsigned int height;

        DiscreteRect() = default;
        DiscreteRect(int x, int y, unsigned int width, unsigned int height) : x(x), y(y), width(width), height(height)
        {
        }

        bool operator==(const DiscreteRect& rhs) const
        {
            return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
        }

        bool operator!=(const DiscreteRect& rhs) const
        {
            return !(rhs == *this);
        }

        /**
         * Returns true if the given coordinates are adjacent to this rectangle.
         * Coordinates are considered adjacent if they touch the perimeter
         * of the rectangle from the outside, including touching at a corner.
         */
        bool isAdjacentTo(int px, int py) const;

        /**
         * Returns the octile distance from the given coordinates
         * to the nearest coordinates that are adjacent to the rectangle.
         */
        OctileDistance octileDistanceToPerimeter(int px, int py) const;
    };
}

namespace std
{
    template <>
    struct hash<rwe::DiscreteRect>
    {
        std::size_t operator()(const rwe::DiscreteRect& r) const noexcept
        {
            std::size_t seed = 0;
            boost::hash_combine(seed, r.x);
            boost::hash_combine(seed, r.y);
            boost::hash_combine(seed, r.width);
            boost::hash_combine(seed, r.height);
            return seed;
        }
    };
}
#endif
