#ifndef RWE_LINE3F_H
#define RWE_LINE3F_H

#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct Line3f
    {
        Vector3f start;
        Vector3f end;

        Line3f(const Vector3f& start, const Vector3f& end);
    };
}

#endif
