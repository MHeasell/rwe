#ifndef RWE_MATH_VECTOR3F_H
#define RWE_MATH_VECTOR3F_H

#include <ostream>
#include <boost/optional.hpp>

namespace rwe
{
    struct Vector3f
    {
        float x;
        float y;
        float z;

        Vector3f() = default;
        Vector3f(float x, float y, float z) noexcept;

        Vector3f& operator+=(const Vector3f& rhs);
        Vector3f& operator-=(const Vector3f& rhs);
        Vector3f& operator*=(float v);
        Vector3f& operator/=(float v);

        Vector3f operator-() const;

        bool operator==(const Vector3f& rhs) const;
        bool operator!=(const Vector3f& rhs) const;

        float lengthSquared() const;
        float length() const;

        float distanceSquared(const Vector3f& rhs) const;
        float distance(const Vector3f& rhs) const;

        void normalize();
        Vector3f normalized();

        float dot(const Vector3f& rhs) const;
        Vector3f cross(const Vector3f& rhs) const;
    };

    Vector3f operator+(const Vector3f& lhs, const Vector3f& rhs);
    Vector3f operator-(const Vector3f& lhs, const Vector3f& rhs);

    Vector3f operator*(const Vector3f& lhs, float rhs);
    Vector3f operator/(const Vector3f& lhs, float rhs);

    std::ostream& operator<<(std::ostream& lhs, const Vector3f& rhs);

    /**
     * Returns the scalar triple product of vectors a, b and c.
     */
    float scalarTriple(const Vector3f& a, const Vector3f& b, const Vector3f& c);

    /**
     * Returns the vector that is closest to v.
     * If either of the vectors a and b are none,
     * the other vector is returned.
     */
    const boost::optional<Vector3f>& closestTo(
        const Vector3f& v,
        const boost::optional<Vector3f>& a,
        const boost::optional<Vector3f>& b);

    /**
     * Returns true if a is closer to v than b.
     * Otherwise, returns false.
     */
    bool isCloserTo(const Vector3f& v, const Vector3f& a, const Vector3f& b);
}

#endif
