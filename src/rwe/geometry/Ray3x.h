#pragma once

#include <rwe/geometry/Line3x.h>
#include <rwe/math/Vector3x.h>

namespace rwe
{
    /** An infinite ray in 3D space. */
    template <typename Val>
    struct Ray3x
    {
        /**
         * Constructs a ray from a line segment.
         * The ray starts at the starting point of the line,
         * and reaches the end of the line at t=1.
         */
        static Ray3x fromLine(const Line3x<Val>& line)
        {
            return Ray3x(line.start, line.end - line.start);
        }

        /** The starting point at the ray, at t = 0. */
        Vector3x<Val> origin;

        /** The direction the ray is travelling. */
        Vector3x<Val> direction;

        Ray3x(const Vector3x<Val>& position, const Vector3x<Val>& direction) : origin(position), direction(direction) {}

        /** Returns the position along the ray at time t. */
        Vector3x<Val> pointAt(float t) const
        {
            return origin + (direction * t);
        }

        /**
         * Returns true if point a is less far along the ray than point b.
         * a and b are both assumed to lie on the ray.
         */
        bool isLessFar(const Vector3x<Val>& a, const Vector3x<Val>& b) const
        {
            return (a - origin).dot(direction) < (b - origin).dot(direction);
        }

        Line3x<Val> toLine() const
        {
            return Line3x<Val>(origin, pointAt(1.0f));
        }
    };
}
