#include "LaserProjectile.h"

namespace rwe
{
    Vector3f LaserProjectile::getBackPosition() const
    {
        auto durationVector = velocity * duration;
        if (durationVector.lengthSquared() < (position - origin).lengthSquared())
        {
            return position - durationVector;
        }
        else
        {
            return origin;
        }
    }
}
