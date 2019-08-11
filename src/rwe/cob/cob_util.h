#pragma once

#include <cstdint>
#include <utility>

namespace rwe
{
    uint32_t packCoords(float x, float z);

    std::pair<float, float> unpackCoords(uint32_t xz);

    int cobAtan(int a, int b);
}
