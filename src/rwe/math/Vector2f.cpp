#include "Vector2f.h"
#include <algorithm>
#include <cmath>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    Vector2f::Vector2f(float x, float y) : x(x), y(y) {}

    Vector2f& Vector2f::operator+=(const Vector2f& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vector2f& Vector2f::operator-=(const Vector2f& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vector2f& Vector2f::operator*=(float v)
    {
        x *= v;
        y *= v;
        return *this;
    }

    Vector2f& Vector2f::operator/=(float v)
    {
        x /= v;
        y /= v;
        return *this;
    }

    float Vector2f::lengthSquared() const
    {
        return (x * x) + (y * y);
    }

    float Vector2f::length() const
    {
        return std::sqrt(lengthSquared());
    }

    float Vector2f::distanceSquared(const Vector2f& rhs) const
    {
        return (rhs - *this).lengthSquared();
    }

    float Vector2f::distance(const Vector2f& rhs) const
    {
        return (rhs - *this).length();
    }

    void Vector2f::normalize()
    {
        float n = length();
        if (n == 0.0f)
        {
            throw std::logic_error("Attempted to normalize a zero-length rwe_math");
        }

        x /= n;
        y /= n;
    }

    Vector2f Vector2f::normalized() const
    {
        float n = length();
        if (n == 0.0f)
        {
            throw std::logic_error("Attempted to normalize a zero-length rwe_math");
        }

        return Vector2f(x / n, y / n);
    }

    float Vector2f::dot(const Vector2f& rhs) const
    {
        return (x * rhs.x) + (y * rhs.y);
    }

    float Vector2f::det(const Vector2f& rhs) const
    {
        return (x * rhs.y) - (y * rhs.x);
    }

    float Vector2f::angleTo(const Vector2f& rhs) const
    {
        return std::atan2(det(rhs), dot(rhs));
    }

    bool Vector2f::operator==(const Vector2f& rhs) const
    {
        return x == rhs.x && y == rhs.y;
    }

    bool Vector2f::operator!=(const Vector2f& rhs) const
    {
        return !(*this == rhs);
    }

    Vector2f operator+(const Vector2f& lhs, const Vector2f& rhs)
    {
        return Vector2f(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    Vector2f operator-(const Vector2f& lhs, const Vector2f& rhs)
    {
        return Vector2f(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    Vector2f operator*(const Vector2f& lhs, float rhs)
    {
        return Vector2f(lhs.x * rhs, lhs.y * rhs);
    }

    Vector2f operator/(const Vector2f& lhs, float rhs)
    {
        return Vector2f(lhs.x / rhs, lhs.y / rhs);
    }

    std::ostream& operator<<(std::ostream& lhs, const Vector2f& rhs)
    {
        lhs << "(" << rhs.x << ", " << rhs.y << ")";
        return lhs;
    }
}
