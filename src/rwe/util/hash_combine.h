#pragma once

#include <cstddef>

namespace rwe
{
    // boost::hash_combine algorithm. The magic constant is the golden
    // ratio's fixed-point representation (2^32 / phi), which spreads
    // sequential inputs across the hash space to minimize collisions.
    inline void hashCombine(std::size_t& seed, std::size_t value)
    {
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}
