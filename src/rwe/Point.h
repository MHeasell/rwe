#ifndef RWE_POINT_H
#define RWE_POINT_H

namespace rwe
{
    struct Point
    {
        int x;
        int y;

        Point() = default;
        Point(int x, int y);
    };
}

#endif
