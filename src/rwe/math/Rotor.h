#pragma once

#include <rwe/math/Bivector3x.h>

namespace rwe
{
    // This code largely lifted from Marc ten Bosch's code sample
    // that accompanies his excellent Rotors explanation.
    //
    // http://marctenbosch.com/quaternions/code.htm
    // http://marctenbosch.com/quaternions/
    template <typename Val>
    struct Rotor3
    {
        // scalar part
        Val a;

        // bivector part
        Val b01;
        Val b02;
        Val b12;

        // default ctor
        Rotor3() = default;
        Rotor3(float a, float b01, float b02, float b12)
            : a(a), b01(b01), b02(b02), b12(b12) {}
        Rotor3(float a, const Bivector3x<Val>& bv)
            : a(a), b01(bv.b01), b02(bv.b02), b12(bv.b12) {}

        // construct the rotor that rotates one vector to another
        static Rotor3 fromVectors(const Vector3x<Val>& vFrom, const Vector3x<Val>& vTo)
        {
            auto a = 1 + vTo.dot(vFrom);
            // the left side of the products have b a, not a b, so flip
            auto minusb = wedge(vTo, vFrom);

            Rotor3 rotor(a, minusb);
            rotor.normalize();
            return rotor;
        }

        // angle+axis, or rather angle+plane
        static Rotor3 fromAnglePlane(Val angleRadian, const Bivector3x<Val>& bvPlane)
        {
            float sina = sin(angleRadian / 2);
            auto a = cos(angleRadian / 2);
            // the left side of the products have b a, not a b
            auto b01 = -sina * bvPlane.b01;
            auto b02 = -sina * bvPlane.b02;
            auto b12 = -sina * bvPlane.b12;

            return Rotor3(a, b01, b02, b12);
        }

        // multiply
        Rotor3 operator*(const Rotor3& q) const
        {
            const Rotor3& p = *this;
            Rotor3 r;

            // here we just expanded the geometric product rules

            r.a = (p.a * q.a)
                - (p.b01 * q.b01) - (p.b02 * q.b02) - (p.b12 * q.b12);

            r.b01 = (p.b01 * q.a) + (p.a * q.b01)
                + (p.b12 * q.b02) - (p.b02 * q.b12);

            r.b02 = (p.b02 * q.a) + (p.a * q.b02)
                - (p.b12 * q.b01) + (p.b01 * q.b12);

            r.b12 = (p.b12 * q.a) + (p.a * q.b12)
                + (p.b02 * q.b01) - (p.b01 * q.b02);

            return r;
        }

        Rotor3 operator*=(const Rotor3& r)
        {
            *this = (*this) * r;
            return *this;
        }

        Vector3x<Val> rotate(const Vector3x<Val>& x) const
        {
            const Rotor3& p = *this;

            // q = P x
            Vector3x<Val> q;
            q.x = (p.a * x.x) + (x.y * p.b01) + (x.z * p.b02);
            q.y = (p.a * x.y) - (x.x * p.b01) + (x.z * p.b12);
            q.z = (p.a * x.z) - (x.x * p.b02) - (x.y * p.b12);

            float q012 = -x.x * p.b12 + x.y * p.b02 - x.z * p.b01; // trivector

            // r = q P*
            Vector3x<Val> r;
            r.x = (p.a * q.x) + (q.y * p.b01) + (q.z * p.b02) - (q012 * p.b12);
            r.y = (p.a * q.y) - (q.x * p.b01) + (q012 * p.b02) + (q.z * p.b12);
            r.z = (p.a * q.z) - (q012 * p.b01) - (q.x * p.b02) - (q.y * p.b12);

            return r;
        }

        Rotor3 rotate(const Rotor3& r) const
        {
            // should unwrap this for efficiency
            return (*this) * r * (*this).reverse();
        }

        // Equivalent to conjugate
        Rotor3 reverse() const
        {
            return Rotor3(a, -b01, -b02, -b12);
        }

        Val lengthsqrd() const
        {
            return Sqr(a) + Sqr(b01) + Sqr(b02) + Sqr(b12);
        }

        Val length() const
        {
            return sqrt(lengthsqrd());
        }

        void normalize()
        {
            auto l = length();
            a /= l;
            b01 /= l;
            b02 /= l;
            b12 /= l;
        }

        Rotor3 normal() const
        {
            Rotor3 r = *this;
            r.normalize();
            return r;
        }
    };

    // geometric product (for reference), produces twice the angle, negative direction
    template <typename Val>
    inline Rotor3<Val> geo(const Vector3x<Val>& a, const Vector3x<Val>& b)
    {
        return Rotor3(a.dot(b), wedge(a, b));
    }
}
