#ifndef RWE_UNITPATH_H
#define RWE_UNITPATH_H

#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct UnitPath
    {
        std::vector<Vector3f> waypoints;
    };
}

#endif
