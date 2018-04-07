#include "DiscreteRect.h"

namespace rwe
{
    bool DiscreteRect::isAdjacentTo(int px, int py) const
    {
        return expand(1).isInteriorPerimeter(px, py);
    }

    bool DiscreteRect::isInteriorPerimeter(int px, int py) const
    {
        int minX = x;
        int maxX = x + width - 1;
        int minY = y;
        int maxY = y + height - 1;

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
        return expand(1).octileDistanceToInterior(px, py);
    }

    OctileDistance DiscreteRect::octileDistanceToInterior(int px, int py) const
    {
        int minX = x;
        int maxX = x + width - 1;
        int minY = y;
        int maxY = y + height - 1;

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
        return DiscreteRect(x - amount, y - amount, width + (2 * amount), height + (2 * amount));
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
