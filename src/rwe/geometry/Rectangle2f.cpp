#include "Rectangle2f.h"

namespace rwe
{

    Rectangle2f Rectangle2f::fromTLBR(float top, float left, float bottom, float right) {
        float x = (left + right) / 2.0f;
        float y = (top + bottom) / 2.0f;
        float halfWidth = (right - left) / 2.0f;
        float halfHeight = (bottom - top) / 2.0f;
        return Rectangle2f(x, y, halfWidth, halfHeight);
    }

    bool Rectangle2f::contains(Vector2f point) {
            auto localPoint = point - position;
            return localPoint.x >= -extents.x
                   && localPoint.x <= extents.x
                   && localPoint.y >= -extents.y
                   && localPoint.y <= extents.y;
    }

    bool Rectangle2f::contains(float x, float y) {
            return contains(Vector2f(x, y));
    }
}
