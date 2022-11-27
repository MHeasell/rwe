#pragma once

#include <boost/functional/hash.hpp>
#include <optional>
#include <rwe/grid/Point.h>
#include <rwe/pathfinding/OctileDistance.h>

namespace rwe
{
    struct DiscreteRect
    {
        static DiscreteRect fromPoints(const Point& p1, const Point& p2);

        int x{0};
        int y{0};
        int width{0};
        int height{0};

        DiscreteRect() = default;
        DiscreteRect(int x, int y, int width, int height) : x(x), y(y), width(width), height(height)
        {
            if (this->width < 0 || this->height < 0)
            {
                throw std::logic_error("invalid parameters");
            }
        }

        bool operator==(const DiscreteRect& rhs) const
        {
            return x == rhs.x && y == rhs.y && width == rhs.width && height == rhs.height;
        }

        bool operator!=(const DiscreteRect& rhs) const
        {
            return !(rhs == *this);
        }

        int left() const
        {
            return x;
        }
        int right() const
        {
            return x + width;
        }
        int top() const
        {
            return y;
        }
        int bottom() const
        {
            return y + width;
        }

        /**
         * Returns true if the given coordinates are adjacent to this rectangle.
         * Coordinates are considered adjacent if they touch the perimeter
         * of the rectangle from the outside, including touching at a corner.
         */
        bool isAdjacentTo(int px, int py) const;

        /**
         * Returns true if the given rectangle is adjacent to this one.
         * Rectangles are considered adjacent if they share an edge or a corner
         * and do not overlap.
         */
        bool isAdjacentTo(const DiscreteRect& rhs) const;

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

        DiscreteRect expand(int amount) const;

        DiscreteRect expand(int dx, int dy) const;

        DiscreteRect translate(int dx, int dy) const;

        bool contains(const Point& p) const;

        /**
         * Returns the intersection of this rectangle and the given rectangle.
         * If the two rectangles do not intersect, returns None.
         */
        std::optional<DiscreteRect> intersection(const DiscreteRect& rhs) const;

        DiscreteRect expandTopLeft(int expandWidth, int expandHeight) const;
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
