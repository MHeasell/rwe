#pragma once

#include <optional>
#include <rapidcheck.h>

namespace rc
{
    namespace gen
    {
        template <typename T>
        Gen<std::optional<T>> optional(Gen<T> gen)
        {
            return gen::map<Maybe<T>>(gen::maybe(std::move(gen)),
                [](Maybe<T>&& m) {
                    return m ? std::optional<T>(std::move(*m))
                             : std::optional<T>();
                });
        }
    }

    template <typename T>
    struct Arbitrary<std::optional<T>>
    {
        static Gen<std::optional<T>> arbitrary()
        {
            return gen::optional(gen::arbitrary<T>());
        }
    };

    template <typename T>
    void showValue(const std::optional<T>& x, std::ostream& os)
    {
        if (x)
        {
            show(*x, os);
        }
        else
        {
            os << "std::nullopt";
        }
    }
}
