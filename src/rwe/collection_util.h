#pragma once

#include <functional>
#include <optional>

namespace rwe
{
    template <typename Container, typename K>
    std::optional<std::reference_wrapper<const typename Container::mapped_type>> tryFind(const Container& c, const K& key)
    {
        auto it = c.find(key);
        if (it == c.end())
        {
            return std::nullopt;
        }

        return std::ref(it->second);
    }

    template <typename Container, typename K>
    std::optional<std::reference_wrapper<typename Container::mapped_type>> tryFind(Container& c, const K& key)
    {
        auto it = c.find(key);
        if (it == c.end())
        {
            return std::nullopt;
        }

        return std::ref(it->second);
    }

    template <typename Container, typename K>
    std::optional<typename Container::mapped_type> tryFindValue(const Container& c, const K& key)
    {
        auto val = tryFind(c, key);
        if (val)
        {
            return val->get();
        }
        return std::nullopt;
    }
}
