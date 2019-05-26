#ifndef RWE_DISCRETERECT_H
#define RWE_DISCRETERECT_H

#include <boost/functional/hash.hpp>
#include <optional>
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

        bool isInteriorPerimeter(int px, int py) const;

        /**
         * Returns true if the top-left of the grid cell represented
         * by the input coordinates touches the line drawn
         * by the rectangle's perimeter in between cells.
         */
        bool topLeftTouchesPerimeter(int px, int py) const;

        /**
         * Returns the octile distance from the given coordinates
         * to the nearest coordinates that are adjacent to the rectangle.
         */
        OctileDistance octileDistanceToPerimeter(int px, int py) const;

        OctileDistance octileDistanceToInterior(int px, int py) const;

        OctileDistance octileDistanceToTopLeftTouching(int px, int py) const;

        DiscreteRect expand(unsigned int amount) const;

        DiscreteRect expand(unsigned int dx, unsigned int dy) const;

        /**
         * Returns the intersection of this rectangle and the given rectangle.
         * If the two rectangles do not intersect, returns None.
         */
        std::optional<DiscreteRect> intersection(const DiscreteRect& rhs) const;
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
