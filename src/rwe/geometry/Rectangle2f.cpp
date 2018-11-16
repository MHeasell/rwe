#include "Rectangle2f.h"
#include <rwe/math/rwe_math.h>

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

    bool Rectangle2f::operator==(const Rectangle2f& rhs) const
    {
        return position == rhs.position && extents == rhs.extents;
    }

    bool Rectangle2f::operator!=(const Rectangle2f& rhs) const
    {
        return !(rhs == *this);
    }

    float Rectangle2f::distanceSquared(const Vector2f& pos) const
    {
        auto toCenter = position - pos;
        auto dX = std::max(0.0f, std::abs(toCenter.x) - extents.x);
        auto dY = std::max(0.0f, std::abs(toCenter.y) - extents.y);
        return (dX * dX) + (dY * dY);
    }

    float Rectangle2f::getScaleToFit(const Rectangle2f& other) const
    {
        auto ratioX = extents.x / other.extents.x;
        auto ratioY = extents.y / other.extents.y;
        return std::min(ratioX, ratioY);
    }

    Rectangle2f Rectangle2f::scaleToFit(const Rectangle2f& other) const
    {
        auto scale = getScaleToFit(other);
        return Rectangle2f(position, other.extents * scale);
    }
}
