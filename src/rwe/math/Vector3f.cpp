#include "Vector3f.h"

namespace rwe
{
    /**
     * Returns the angle between this vector and the rhs.
     * The returned angle is positive if the rotation to the rhs vector
     * would be anti-clockwise when viewed from the end of the supplied normal.
     * The range of the return value is -PI <= v < PI.
     */
    float angleTo(const Vector3f& lhs, const Vector3f& rhs, const Vector3f& normal)
    {
        auto cosAngle = lhs.dot(rhs) / (lhs.length() * rhs.length());
        // acos is only defined between -1 and 1
        // but float rounding error can nudge us over.
        // clamp to prevent this from causing problems.
        auto angle = std::acos(std::clamp(cosAngle, -1.0f, 1.0f));
        auto det = determinant(lhs, rhs, normal);
        return angle * (det > 0.0f ? 1.0f : -1.0f);
    }
}
