#ifndef RWE_NETWORK_UTIL_H
#define RWE_NETWORK_UTIL_H

#include <rwe/SceneTime.h>
#include <rwe/rwe_time.h>


namespace rwe
{
    float ema(float val, float average, float alpha);
}

#endif
