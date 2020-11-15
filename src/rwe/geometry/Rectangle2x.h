#pragma once

#include <algorithm>
#include <rwe/math/Vector2x.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    template <typename Val>
    struct Rectangle2x
    {
        static Rectangle2x fromTLBR(Val top, Val left, Val bottom, Val right)
        {
            auto x = (left + right) / Val(2);
            auto y = (top + bottom) / Val(2);
            auto halfWidth = (right - left) / Val(2);
            auto halfHeight = (bottom - top) / Val(2);
            return Rectangle2x(x, y, halfWidth, halfHeight);
        }

        static Rectangle2x fromTopLeft(Val x, Val y, Val width, Val height)
        {
            Val halfHeight = height / Val(2);
            Val halfWidth = width / Val(2);
            return Rectangle2x(x + halfWidth, y + halfHeight, halfWidth, halfHeight);
        }

        Vector2x<Val> position;
        Vector2x<Val> extents;

        Rectangle2x(Vector2x<Val> position, Vector2x<Val> extents) : position(position), extents(extents) {}
        Rectangle2x(Val x, Val y, Val halfWidth, Val halfHeight) : position(x, y), extents(halfWidth, halfHeight) {}

        bool operator==(const Rectangle2x& rhs) const
        {
            return position == rhs.position && extents == rhs.extents;
        }

        bool operator!=(const Rectangle2x& rhs) const
        {
            return !(rhs == *this);
        }

        bool contains(Vector2x<Val> point) const
        {
            auto localPoint = point - position;
            return localPoint.x >= -extents.x
                && localPoint.x <= extents.x
                && localPoint.y >= -extents.y
                && localPoint.y <= extents.y;
        }

        bool contains(Val x, Val y) const
        {
            return contains(Vector2x<Val>(x, y));
        }

        Val left() const
        {
            return position.x - extents.x;
        }

        Val right() const
        {
            return position.x + extents.x;
        }

        Val top() const
        {
            return position.y - extents.y;
        }

        Val bottom() const
        {
            return position.y + extents.y;
        }

        Vector2x<Val> topLeft() const
        {
            return position - extents;
        }

        Vector2x<Val> topRight() const
        {
            return Vector2x<Val>(right(), top());
        }

        Vector2x<Val> bottomLeft() const
        {
            return Vector2x<Val>(left(), bottom());
        }

        Vector2x<Val> bottomRight() const
        {
            return position + extents;
        }

        Val width() const
        {
            return extents.x * Val(2);
        }
        Val height() const
        {
            return extents.y * Val(2);
        }

        Val distanceSquared(const Vector2x<Val>& pos) const
        {
            auto toCenter = position - pos;
            auto dX = std::max(Val(0), rweAbs(toCenter.x) - extents.x);
            auto dY = std::max(Val(0), rweAbs(toCenter.y) - extents.y);
            return (dX * dX) + (dY * dY);
        }

        /**
         * Returns the scale factor you would need to multiply
         * the other rectangle's dimensions by
         * in order for it to fit proportionally inside this one,
         * touching the edges.
         */
        Val getScaleToFit(const Rectangle2x& other) const
        {
            auto ratioX = extents.x / other.extents.x;
            auto ratioY = extents.y / other.extents.y;
            return std::min(ratioX, ratioY);
        }

        /**
         * Returns a copy of target rectangle
         * positioned in the centre of this rectangle
         * and scaled to fit proportionally exactly inside this rectangle.
         */
        Rectangle2x scaleToFit(const Rectangle2x& other) const
        {
            auto scale = getScaleToFit(other);
            return Rectangle2x(position, other.extents * scale);
        }

        Vector2x<Val> clamp(const Vector2x<Val>& v) const
        {
            return Vector2x<Val>(
                std::clamp(v.x, left(), right()),
                std::clamp(v.y, top(), bottom()));
        }
    };
}
