#include "fixed_point.h"

namespace rwe
{
    int toFixedPoint(float val)
    {
        return static_cast<unsigned int>(val * 65536.0f);
    }

    float fromFixedPoint(int val)
    {
        return static_cast<float>(val) / 65536.0f;
    }
}
