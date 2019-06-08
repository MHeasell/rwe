#include "DiscreteRect.h"
#include <algorithm>

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
        return DiscreteRect(x - 1, y - 1, width + 1, height + 1).topLeftTouchesPerimeter(px, py);
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
            auto distanceX = static_cast<unsigned int>(minX - px);

            // top-left corner
            if (py <= minY)
            {
                auto distanceY = static_cast<unsigned int>(minY - py);
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // bottom-left corner
            if (py >= maxY)
            {
                auto distanceY = static_cast<unsigned int>(py - maxY);
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // left edge
            return OctileDistance(distanceX, 0);
        }

        // point is right of the rectangle
        if (px >= maxX)
        {
            auto distanceX = static_cast<unsigned int>(px - maxX);

            // top-right corner
            if (py <= minY)
            {
                auto distanceY = static_cast<unsigned int>(minY - py);
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // bottom-right corner
            if (py >= maxY)
            {
                auto distanceY = static_cast<unsigned int>(py - maxY);
                return OctileDistance::fromXAndY(distanceX, distanceY);
            }

            // right edge
            return OctileDistance(distanceX, 0);
        }

        // top edge
        if (py <= minY)
        {
            auto distanceY = static_cast<unsigned int>(minY - py);
            return OctileDistance(distanceY, 0);
        }

        // bottom edge
        if (py >= maxY)
        {
            auto distanceY = static_cast<unsigned int>(py - maxY);
            return OctileDistance(distanceY, 0);
        }

        // inside
        auto distance = std::min<unsigned int>(
            {static_cast<unsigned int>(px - minX),
                static_cast<unsigned int>(maxX - px),
                static_cast<unsigned int>(py - minY),
                static_cast<unsigned int>(maxY - py)});
        return OctileDistance(distance, 0);
    }

    DiscreteRect DiscreteRect::expand(unsigned int amount) const
    {
        return expand(amount, amount);
    }

    DiscreteRect DiscreteRect::expand(unsigned int dx, unsigned int dy) const
    {
        return DiscreteRect(x - dx, y - dy, width + (2 * dx), height + (2 * dy));
    }

    std::optional<DiscreteRect> DiscreteRect::intersection(const DiscreteRect& rhs) const
    {
        auto left = std::max(x, rhs.x);
        auto top = std::max(y, rhs.y);

        auto right = std::min(x + static_cast<int>(width), rhs.x + static_cast<int>(rhs.width));
        auto bottom = std::min(y + static_cast<int>(height), rhs.y + static_cast<int>(rhs.height));

        auto intersectWidth = right - left;
        auto intersectHeight = bottom - top;

        if (intersectWidth <= 0 || intersectHeight <= 0)
        {
            return std::nullopt;
        }

        return DiscreteRect(left, top, static_cast<unsigned int>(intersectWidth), static_cast<unsigned int>(intersectHeight));
    }
}
