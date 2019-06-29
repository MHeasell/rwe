#include "network_util.h"

namespace rwe
{
    float ema(float val, float average, float alpha)
    {
        return (alpha * val) + ((1.0f - alpha) * average);
    }

}
