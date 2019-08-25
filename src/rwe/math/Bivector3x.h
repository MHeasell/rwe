#pragma once

#include <rwe/math/Vector3x.h>

namespace rwe
{
    // This code largely lifted from Marc ten Bosch's code sample
    // that accompanies his excellent Rotors explanation.
    //
    // http://marctenbosch.com/quaternions/code.htm
    // http://marctenbosch.com/quaternions/
    template <typename Val>
    struct Bivector3x
    {
        Val b01;
        Val b02;
        Val b12;

        Bivector3x(Val b01, Val b02, Val b12)
            : b01(b01), b02(b02), b12(b12) {}
    };

    // Wedge product
    template <typename Val>
    inline Bivector3x<Val> wedge(const Vector3x<Val>& u, const Vector3x<Val>& v)
    {
        Bivector3x ret(
            (u.x * v.y) - (u.y * v.x), // XY
            (u.x * v.z) - (u.z * v.x), // XZ
            (u.y * v.z) - (u.z * v.y)  // YZ
        );
        return ret;
    }
}
