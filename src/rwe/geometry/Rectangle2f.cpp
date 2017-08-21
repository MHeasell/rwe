#include "Rectangle2f.h"

namespace rwe
{

    Rectangle2f Rectangle2f::fromTLBR(float top, float left, float bottom, float right)
    {
        float x = (left + right) / 2.0f;
        float y = (top + bottom) / 2.0f;
        float halfWidth = (right - left) / 2.0f;
        float halfHeight = (bottom - top) / 2.0f;
        return Rectangle2f(x, y, halfWidth, halfHeight);
    }

    Rectangle2f Rectangle2f::fromTopLeft(float x, float y, float width, float height)
    {
        float halfHeight = height / 2.0f;
        float halfWidth = width / 2.0f;
        return Rectangle2f(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
    }

    bool Rectangle2f::contains(Vector2f point) const
    {
        auto localPoint = point - position;
        return localPoint.x >= -extents.x
            && localPoint.x <= extents.x
            && localPoint.y >= -extents.y
            && localPoint.y <= extents.y;
    }

    bool Rectangle2f::contains(float x, float y) const
    {
        return contains(Vector2f(x, y));
    }

    float Rectangle2f::left() const
    {
        return position.x - extents.x;
    }

    float Rectangle2f::right() const
    {
        return position.x + extents.x;
    }

    float Rectangle2f::top() const
    {
        return position.y - extents.y;
    }

    float Rectangle2f::bottom() const
    {
        return position.y + extents.y;
    }

    Vector2f Rectangle2f::topLeft() const
    {
        return position - extents;
    }

    Vector2f Rectangle2f::bottomRight() const
    {
        return position + extents;
    }

    Vector2f Rectangle2f::topRight() const
    {
        return Vector2f(right(), top());
    }

    Vector2f Rectangle2f::bottomLeft() const
    {
        return Vector2f(left(), bottom());
    }

    float Rectangle2f::width() const
    {
        return extents.x * 2;
    }

    float Rectangle2f::height() const
    {
        return extents.y * 2;
    }
}
