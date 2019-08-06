#pragma once

#include <rwe/math/Vector3x.h>

namespace rwe
{
    template <typename Val>
    struct Line3x
    {
        Vector3x<Val> start;
        Vector3x<Val> end;

        Line3x(const Vector3x<Val>& start, const Vector3x<Val>& end) : start(start), end(end)
        {
        }
    };
}
