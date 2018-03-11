#ifndef RWE_LASERPROJECTILE_H
#define RWE_LASERPROJECTILE_H

#include <rwe/geometry/Line3f.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct LaserProjectile
    {
        Vector3f position;

        Vector3f origin;

        /** Velocity in game pixels/tick */
        Vector3f velocity;

        /** Duration in ticks */
        float duration;

        Vector3f getBackPosition() const;
    };
}

#endif
