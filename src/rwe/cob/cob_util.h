#ifndef RWE_COB_UTIL_H
#define RWE_COB_UTIL_H

#include <cstdint>
#include <utility>

namespace rwe
{
    uint32_t packCoords(float x, float z);

    std::pair<float, float> unpackCoords(uint32_t xz);
}

#endif
