#pragma once

#include <deque>
#include <optional>
#include <rwe/util/OpaqueId.h>
#include <utility>

namespace rwe
{
    template <typename T, typename IdTag>
    class SimpleVectorMap
    {
    public:
        using const_iterator = typename std::vector<T>::const_iterator;

    private:
        using Id = OpaqueId<unsigned int, IdTag>;

        std::vector<T> vec;

    public:
        template <typename... Args>
        Id emplace(Args&&... args)
        {
            auto id = Id(vec.size());
            vec.emplace_back(std::forward<Args>(args)...);
            return id;
        }

        Id insert(const T& item)
        {
            auto id = Id(vec.size());
            vec.push_back(item);
            return id;
        }

        Id insert(T&& item)
        {
            auto id = Id(vec.size());
            vec.push_back(std::move(item));
            return id;
        }

        std::optional<std::reference_wrapper<T>> tryGet(Id id)
        {
            if (id.value >= vec.size())
            {
                return std::nullopt;
            }

            return vec[id.value];
        }

        std::optional<std::reference_wrapper<const T>> tryGet(Id id) const
        {
            if (id.value >= vec.size())
            {
                return std::nullopt;
            }

            return vec[id.value];
        }

        T& get(Id id)
        {
            return vec.at(id.value);
        }

        const T& get(Id id) const
        {
            return vec.at(id.value);
        }

        Id getNextId() const
        {
            return Id(vec.size());
        }
    };
}
