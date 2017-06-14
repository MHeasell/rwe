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

        float lengthSquared() const;
        float length() const;

        float distanceSquared(const Vector2f& rhs);
        float distance(const Vector2f& rhs);

        void normalize();
        Vector2f normalized();

        float dot(const Vector2f& rhs) const;
    };

    Vector2f operator+(const Vector2f& lhs, const Vector2f& rhs);
    Vector2f operator-(const Vector2f& lhs, const Vector2f& rhs);

    Vector2f operator*(const Vector2f& lhs, float rhs);
    Vector2f operator/(const Vector2f& lhs, float rhs);

    std::ostream& operator<<(std::ostream& lhs, const Vector2f& rhs);
}

#endif
