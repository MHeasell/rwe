#pragma once

#include <algorithm>
#include <cmath>
#include <optional>
#include <ostream>
#include <rwe/math/Vector2x.h>

namespace rwe
{
    template <typename Val>
    struct Vector3x
    {
        Val x;
        Val y;
        Val z;

        Vector3x() = default;
        Vector3x(Val x, Val y, Val z) noexcept : x(x), y(y), z(z) {}

        bool operator==(const Vector3x& rhs) const
        {
            return x == rhs.x && y == rhs.y && z == rhs.z;
        }

        bool operator!=(const Vector3x& rhs) const
        {
            return !(rhs == *this);
        }


        Vector3x operator+(const Vector3x& rhs) const
        {
            return Vector3x(x + rhs.x, y + rhs.y, z + rhs.z);
        }

        Vector3x& operator+=(const Vector3x& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }

        Vector3x operator-(const Vector3x& rhs) const
        {
            return Vector3x(x - rhs.x, y - rhs.y, z - rhs.z);
        }

        Vector3x& operator-=(const Vector3x& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }

        Vector3x operator-() const
        {
            return Vector3x(-x, -y, -z);
        }

        Vector3x operator*(const Vector3x& rhs) const
        {
            return Vector3x(x * rhs.x, y * rhs.y, z * rhs.z);
        }

        Vector3x operator*(Val rhs) const
        {
            return Vector3x(x * rhs, y * rhs, z * rhs);
        }

        Vector3x& operator*=(Val v)
        {
            x *= v;
            y *= v;
            z *= v;
            return *this;
        }

        Vector3x operator/(Val rhs) const
        {
            return Vector3x(x / rhs, y / rhs, z / rhs);
        }

        Vector3x& operator/=(Val v)
        {
            x /= v;
            y /= v;
            z /= v;
            return *this;
        }

        Val lengthSquared() const
        {
            return (x * x) + (y * y) + (z * z);
        }

        Val length() const
        {
            return rweSqrt(lengthSquared());
        }

        Val distanceSquared(const Vector3x& rhs) const
        {
            return (rhs - *this).lengthSquared();
        }

        Val distance(const Vector3x& rhs) const
        {
            return (rhs - *this).length();
        }

        void normalize()
        {
            Val n = length();
            if (n == Val(0))
            {
                throw std::logic_error("Attempted to normalize a zero-length vector");
            }

            x /= n;
            y /= n;
            z /= n;
        }

        Vector3x normalized() const
        {
            Val n = length();
            if (n == Val(0))
            {
                throw std::logic_error("Attempted to normalize a zero-length vector");
            }

            return Vector3x(x / n, y / n, z / n);
        }

        Vector3x normalizedOr(const Vector3x& defaultValue) const
        {
            auto n = length();
            if (n == Val(0))
            {
                return defaultValue;
            }

            return Vector3x(x / n, y / n, z / n);
        }

        Val dot(const Vector3x& rhs) const
        {
            return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
        }

        Vector3x cross(const Vector3x& rhs) const
        {
            return Vector3x(
                (y * rhs.z) - (z * rhs.y),
                (z * rhs.x) - (x * rhs.z),
                (x * rhs.y) - (y * rhs.x));
        }

        Vector2x<Val> xy() const
        {
            return Vector2x(x, y);
        }

        Vector2x<Val> xz() const
        {
            return Vector2x(x, z);
        }
    };

    template <typename Val>
    std::ostream& operator<<(std::ostream& lhs, const Vector3x<Val>& rhs)
    {
        lhs << "(" << rhs.x << ", " << rhs.y << ", " << rhs.z << ")";
        return lhs;
    }

    /**
     * Returns the scalar triple product of vectors a, b and c.
     */
    template <typename Val>
    Val scalarTriple(const Vector3x<Val>& a, const Vector3x<Val>& b, const Vector3x<Val>& c)
    {
        return a.cross(b).dot(c);
    }

    /**
     * Returns the vector that is closest to v.
     * If either of the vectors a and b are none,
     * the other vector is returned.
     */
    template <typename Val>
    const std::optional<Vector3x<Val>>& closestTo(
        const Vector3x<Val>& v,
        const std::optional<Vector3x<Val>>& a,
        const std::optional<Vector3x<Val>>& b)
    {
        if (!a)
        {
            return b;
        }

        if (!b)
        {
            return a;
        }

        if ((*a - v).lengthSquared() < (*b - v).lengthSquared())
        {
            return a;
        }

        return b;
    }

    /**
     * Returns true if a is closer to v than b.
     * Otherwise, returns false.
     */
    template <typename Val>
    bool isCloserTo(const Vector3x<Val>& v, const Vector3x<Val>& a, const Vector3x<Val>& b)
    {
        return (a - v).lengthSquared() < (b - v).lengthSquared();
    }

    template <typename Val>
    Val determinant(const Vector3x<Val>& v1, const Vector3x<Val>& v2, const Vector3x<Val>& v3)
    {
        auto a = v1.x;
        auto b = v2.x;
        auto c = v3.x;
        auto d = v1.y;
        auto e = v2.y;
        auto f = v3.y;
        auto g = v1.z;
        auto h = v2.z;
        auto i = v3.z;

        return (a * e * i) + (b * f * g) + (c * d * h) - (c * e * g) - (b * d * i) - (a * f * h);
    }

    /**
     * Returns true if vectors A and B are closer than epsilon units apart
     * as measured by straight-line distance between the two vectors.
     */
    template <typename Val>
    bool areCloserThan(const Vector3x<Val>& a, const Vector3x<Val>& b, Val epsilon)
    {
        auto distanceSq = a.distanceSquared(b);
        return distanceSq < (epsilon * epsilon);
    }
}
