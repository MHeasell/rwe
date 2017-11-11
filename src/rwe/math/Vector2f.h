#ifndef RWE_MATH_VECTOR2F_H
#define RWE_MATH_VECTOR2F_H

#include <ostream>

namespace rwe
{
    struct Vector2f
    {
        float x;
        float y;

        Vector2f() = default;
        Vector2f(float x, float y);

        Vector2f& operator+=(const Vector2f& rhs);
        Vector2f& operator-=(const Vector2f& rhs);
        Vector2f& operator*=(float v);
        Vector2f& operator/=(float v);

        bool operator==(const Vector2f& rhs) const;
        bool operator!=(const Vector2f& rhs) const;

        float lengthSquared() const;
        float length() const;

        float distanceSquared(const Vector2f& rhs);
        float distance(const Vector2f& rhs);

        void normalize();
        Vector2f normalized();

        float dot(const Vector2f& rhs) const;

        /**
         * Returns the angle that you would need to rotate this vector by anticlockwise
         * in order to reach the angle of the given vector.
         * The range of the return value is -PI <= v < PI.
         */
        float angleTo(const Vector2f& rhs) const;
    };

    Vector2f operator+(const Vector2f& lhs, const Vector2f& rhs);
    Vector2f operator-(const Vector2f& lhs, const Vector2f& rhs);

    Vector2f operator*(const Vector2f& lhs, float rhs);
    Vector2f operator/(const Vector2f& lhs, float rhs);

    std::ostream& operator<<(std::ostream& lhs, const Vector2f& rhs);
}

#endif
