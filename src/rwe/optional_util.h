#ifndef RWE_OPTIONAL_UTIL_H
#define RWE_OPTIONAL_UTIL_H

#include <functional>
#include <optional>

namespace rwe
{
    template <typename T>
    std::optional<T> refToCopy(const std::optional<std::reference_wrapper<const T>> opt)
    {
        if (!opt)
        {
            return std::nullopt;
        }

        return opt->get();
    }
}

#endif
