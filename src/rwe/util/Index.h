#pragma once
#include <cstddef>

namespace rwe
{
    using Index = std::ptrdiff_t;

    template <typename T>
    [[nodiscard]] inline Index getSize(const T& container)
    {
        return static_cast<Index>(container.size());
    }
}
