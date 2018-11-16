#ifndef RWE_RECTANGLE2F_H
#define RWE_RECTANGLE2F_H

#include <rwe/math/Vector2f.h>

namespace rwe
{
    struct Rectangle2f
    {
        Vector2f position;
        Vector2f extents;

        Rectangle2f(Vector2f position, Vector2f extents) : position(position), extents(extents) {}
        Rectangle2f(float x, float y, float halfWidth, float halfHeight) : position(x, y), extents(halfWidth, halfHeight) {}

        static Rectangle2f fromTLBR(float top, float left, float bottom, float right);
        static Rectangle2f fromTopLeft(float x, float y, float width, float height);

        bool operator==(const Rectangle2f& rhs) const;

        bool operator!=(const Rectangle2f& rhs) const;

        bool contains(Vector2f point) const;

        bool contains(float x, float y) const;

        float left() const;
        float right() const;
        float top() const;
        float bottom() const;

        Vector2f topLeft() const;
        Vector2f topRight() const;
        Vector2f bottomLeft() const;
        Vector2f bottomRight() const;

        float width() const;
        float height() const;

        float distanceSquared(const Vector2f& pos) const;

        /**
         * Returns the scale factor you would need to multiply
         * the other rectangle's dimensions by
         * in order for it to fit proportionally inside this one,
         * touching the edges.
         */
        float getScaleToFit(const Rectangle2f& other) const;

        /**
         * Returns a copy of target rectangle
         * positioned in the centre of this rectangle
         * and scaled to fit proportionally exactly inside this rectangle.
         */
        Rectangle2f scaleToFit(const Rectangle2f& other) const;

        Vector2f clamp(const Vector2f& v) const;
    };
}


#endif
