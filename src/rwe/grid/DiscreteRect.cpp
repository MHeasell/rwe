#include "DiscreteRect.h"
#include <algorithm>
#include <stdexcept>

namespace rwe
{
    DiscreteRect DiscreteRect::fromPoints(const Point& p1, const Point& p2)
    {
        auto x = std::minmax(p1.x, p2.x);
        auto y = std::minmax(p1.y, p2.y);
        return DiscreteRect(x.first, y.first, x.second - x.first + 1, y.second - y.first + 1);
    }

    bool DiscreteRect::isAdjacentTo(int px, int py) const
    {
        return expandTopLeft(1, 1).topLeftTouchesPerimeter(px, py);
    }

    bool DiscreteRect::isAdjacentTo(const DiscreteRect& rhs) const
    {
        return expandTopLeft(rhs.width, rhs.height).topLeftTouchesPerimeter(rhs.x, rhs.y);
    }

    bool DiscreteRect::isInteriorPerimeter(int px, int py) const
    {
        if (width == 0 || height == 0)
        {
            return false;
        }

        return DiscreteRect(x, y, width - 1, height - 1).topLeftTouchesPerimeter(px, py);
    }

    bool DiscreteRect::topLeftTouchesPerimeter(int px, int py) const
    {
        int minX = x;
        int maxX = x + width;
        int minY = y;
        int maxY = y + height;

        if (py == minY || py == maxY)
        {
            if (px >= minX && px <= maxX)
            {
                return true;
            }
        }

        if (px == minX || px == maxX)
        {
            if (py > minY && py < maxY)
            {
                return true;
            }
        }

        return false;
    }

    OctileDistance DiscreteRect::octileDistanceToPerimeter(int px, int py) const
    {
        return DiscreteRect(x - 1, y - 1, width + 1, height + 1).octileDistanceToTopLeftTouching(px, py);
    }

    OctileDistance DiscreteRect::octileDistanceToInterior(int px, int py) const
    {
        if (width == 0 || height == 0)
        {
            throw std::logic_error("Rectangle has no interior");
        }

        return DiscreteRect(x, y, width - 1, height - 1).octileDistanceToTopLeftTouching(px, py);
    }

    OctileDistance DiscreteRect::octileDistanceToTopLeftTouching(int px, int py) const
    {
        int minX = x;
        int maxX = x + width;
        int minY = y;
        int maxY = y + height;

        // point is left of the rectangle
        if (px <= minX)
        {
            auto distanceX = minX - px;

            // top-left corner
            if (py <= minY)
            {
                auto distanceY = minY - py;
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // bottom-left corner
            if (py >= maxY)
            {
                auto distanceY = py - maxY;
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // left edge
            return OctileDistance(distanceX, 0);
        }

        // point is right of the rectangle
        if (px >= maxX)
        {
            auto distanceX = px - maxX;

            // top-right corner
            if (py <= minY)
            {
                auto distanceY = minY - py;
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // bottom-right corner
            if (py >= maxY)
            {
                auto distanceY = py - maxY;
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // right edge
            return OctileDistance(distanceX, 0);
        }

        // top edge
        if (py <= minY)
        {
            auto distanceY = minY - py;
            return OctileDistance(distanceY, 0);
        }

        // bottom edge
        if (py >= maxY)
        {
            auto distanceY = py - maxY;
            return OctileDistance(distanceY, 0);
        }

        // inside
        auto distance = std::min<int>(
            {px - minX,
             maxX - px,
             py - minY,
             maxY - py});
        return OctileDistance(distance, 0);
    }

    DiscreteRect DiscreteRect::expand(int amount) const
    {
        return expand(amount, amount);
    }

    DiscreteRect DiscreteRect::expand(int dx, int dy) const
    {
        return DiscreteRect(x - dx, y - dy, width + (2 * dx), height + (2 * dy));
    }

    std::optional<DiscreteRect> DiscreteRect::intersection(const DiscreteRect& rhs) const
    {
        auto left = std::max(x, rhs.x);
        auto top = std::max(y, rhs.y);

        auto right = std::min(x + width, rhs.x + rhs.width);
        auto bottom = std::min(y + height, rhs.y + rhs.height);

        auto intersectWidth = right - left;
        auto intersectHeight = bottom - top;

        if (intersectWidth <= 0 || intersectHeight <= 0)
        {
            return std::nullopt;
        }

        return DiscreteRect(left, top, intersectWidth, intersectHeight);
    }

    DiscreteRect DiscreteRect::translate(int dx, int dy) const
    {
        return DiscreteRect(x + dx, y + dy, width, height);
    }

    bool DiscreteRect::contains(const Point& p) const
    {
        return p.x >= x && p.y >= y && p.x < (x + width) && p.y < (y + height);
    }

    DiscreteRect DiscreteRect::expandTopLeft(int expandWidth, int expandHeight) const
    {
        return DiscreteRect(
            x - expandWidth,
            y - expandHeight,
            width + expandWidth,
            height + expandHeight);
    }
}
