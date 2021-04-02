#pragma once

#include <variant>

namespace rwe
{
    template <typename... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <typename... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template <typename Variant, typename... Ts>
    constexpr decltype(auto) match(Variant&& variant, Ts&&... funcs)
    {
        return std::visit(
            overloaded{std::forward<Ts>(funcs)...},
            std::forward<Variant>(variant));
    }
}
