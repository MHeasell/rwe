#pragma once

#include <ranges>

namespace rwe
{
    template <typename Range, typename Chooser>
    auto choose(Range r, Chooser c)
    {
        return r
            | std::views::transform(c)
            | std::views::filter([](const auto& e) { return e.has_value(); })
            | std::views::transform([](const auto& e) { return *e; });
    }
}
