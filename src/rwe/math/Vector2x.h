#ifndef RWE_MATH_VECTOR2X_H
#define RWE_MATH_VECTOR2X_H

#include <cmath>
#include <ostream>

namespace rwe
{
    template <typename Val>
    struct Vector2x
    {
        Val x;
        Val y;

        Vector2x() = default;
        Vector2x(Val x, Val y) : x(x), y(y) {}

        bool operator==(const Vector2x& rhs) const
        {
            return x == rhs.x && y == rhs.y;
        }

        bool operator!=(const Vector2x& rhs) const
        {
            return !(*this == rhs);
        }


        Vector2x operator+(const Vector2x& rhs) const
        {
            return Vector2x(x + rhs.x, y + rhs.y);
        }

        Vector2x& operator+=(const Vector2x& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        Vector2x operator-(const Vector2x& rhs) const
        {
            return Vector2x(x - rhs.x, y - rhs.y);
        }

        Vector2x& operator-=(const Vector2x& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        Vector2x operator*(Val rhs) const
        {
            return Vector2x(x * rhs, y * rhs);
        }

        Vector2x& operator*=(Val v)
        {
            x *= v;
            y *= v;
            return *this;
        }

        Vector2x operator/(Val rhs) const
        {
            return Vector2x(x / rhs, y / rhs);
        }

        Vector2x& operator/=(Val v)
        {
            x /= v;
            y /= v;
            return *this;
        }

        Val lengthSquared() const
        {
            return (x * x) + (y * y);
        }

        Val length() const
        {
            return std::sqrt(lengthSquared());
        }

        Val distanceSquared(const Vector2x& rhs) const
        {
            return (rhs - *this).lengthSquared();
        }

        Val distance(const Vector2x& rhs) const
        {
            return (rhs - *this).length();
        }

        void normalize()
        {
            Val n = length();
            if (n == 0.0f)
            {
                throw std::logic_error("Attempted to normalize a zero-length rwe_math");
            }

            x /= n;
            y /= n;
        }

        Vector2x normalized() const
        {
            Val n = length();
            if (n == 0.0f)
            {
                throw std::logic_error("Attempted to normalize a zero-length rwe_math");
            }

            return Vector2x(x / n, y / n);
        }

        Val dot(const Vector2x& rhs) const
        {
            return (x * rhs.x) + (y * rhs.y);
        }

        /**
         * Computes the determinant of the two vectors
         * as though they were columns of a 2x2 matrix.
         */
        Val det(const Vector2x& rhs) const
        {
            return (x * rhs.y) - (y * rhs.x);
        }

        /**
         * Returns the angle that you would need to rotate this vector by anticlockwise
         * in order to reach the angle of the given vector.
         * The range of the return value is -PI <= v < PI.
         */
        Val angleTo(const Vector2x& rhs) const
        {
            return std::atan2(det(rhs), dot(rhs));
        }
    };

    template <typename Val>
    std::ostream& operator<<(std::ostream& lhs, const Vector2x<Val>& rhs)
    {
        lhs << "(" << rhs.x << ", " << rhs.y << ")";
        return lhs;
    }
}

#endif
