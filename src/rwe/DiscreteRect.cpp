#include "DiscreteRect.h"

namespace rwe
{
    bool DiscreteRect::isAdjacentTo(int px, int py) const
    {
        int minX = x - 1;
        int maxX = x + width;
        int minY = y - 1;
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
        int minX = x - 1;
        int maxX = x + width;
        int minY = y - 1;
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
}
