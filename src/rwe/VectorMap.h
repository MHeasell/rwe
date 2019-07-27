#ifndef RWE_VECTORMAP_H
#define RWE_VECTORMAP_H

#include <cassert>
#include <deque>
#include <functional>
#include <optional>
#include <rwe/OpaqueId.h>
#include <rwe/overloaded.h>
#include <utility>
#include <variant>

namespace rwe
{
    template <typename T, typename IdTag>
    class VectorMap
    {
    private:
        using Id = OpaqueId<unsigned int, IdTag>;
        struct IndexTag;
        using Index = OpaqueId<unsigned int, IndexTag>;
        struct GenerationTag;
        using Generation = OpaqueId<unsigned int, GenerationTag>;

        struct FreeEntry
        {
            std::optional<Index> nextIndex;
            Id id;
            FreeEntry(Id id, const std::optional<Index>& nextIndex) : nextIndex(nextIndex), id(id) {}
        };

        using OccupiedEntry = std::pair<Id, T>;
        using Entry = std::variant<FreeEntry, OccupiedEntry>;

    public:
        template <typename Val, typename Ref, typename Ptr, typename UnderlyingIt>
        class VectorMapIter
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Val;
            using reference = Ref;
            using pointer = Ptr;
            using difference_type = int; // not used

        private:
            UnderlyingIt it;
            UnderlyingIt end;

        public:
            VectorMapIter(
                UnderlyingIt it,
                UnderlyingIt end) : it(std::move(it)), end(std::move(end))
            {
            }

            VectorMapIter& operator++()
            {
                assert(it != end);
                it = firstValid(++it, end);
                return *this;
            }

            VectorMapIter operator++(int)
            {
                VectorMapIter tmp(*this);
                ++*this;
                return tmp;
            }

            reference operator*() const
            {
                return std::get<std::remove_const_t<value_type>>(*it);
            }

            pointer operator->() const
            {
                return &std::get<std::remove_const_t<value_type>>(*it);
            }

            bool operator==(const VectorMapIter& rhs) const
            {
                return it == rhs.it && end == rhs.end;
            }

            bool operator!=(const VectorMapIter& rhs) const
            {
                return !(rhs == *this);
            }
        };

        using iterator = VectorMapIter<OccupiedEntry, OccupiedEntry&, OccupiedEntry*, typename std::deque<Entry>::iterator>;
        using const_iterator = VectorMapIter<const OccupiedEntry, const OccupiedEntry&, const OccupiedEntry*, typename std::deque<Entry>::const_iterator>;

        using key_type = Id;
        using mapped_type = T;
        using value_type = OccupiedEntry;

    private:
        std::deque<Entry> vec;
        std::optional<Index> firstFreeSlotIndex;

    public:
        template <typename... Args>
        Id emplace(Args&&... args)
        {
            if (firstFreeSlotIndex)
            {
                auto index = *firstFreeSlotIndex;
                auto& entry = vec[index.value];
                const auto& freeEntry = std::get<FreeEntry>(vec[index.value]);
                firstFreeSlotIndex = freeEntry.nextIndex;
                auto newId = nextGeneration(freeEntry.id);
                entry = std::make_pair(newId, T(std::forward<Args>(args)...));
                return newId;
            }
            else
            {
                auto id = makeId(Index(vec.size()));
                vec.emplace_back(std::make_pair(id, T(std::forward<Args>(args)...)));
                return id;
            }
        }

        void remove(Id id)
        {
            auto index = extractIndex(id);

            if (index.value >= vec.size())
            {
                throw std::runtime_error("Member with given ID does not exist");
            }

            auto& entry = vec[index.value];
            if (const auto f = std::get_if<OccupiedEntry>(&entry); f != nullptr && f->first == id)
            {
                entry = FreeEntry(id, firstFreeSlotIndex);
                firstFreeSlotIndex = index;
                return;
            }

            throw std::runtime_error("Member with given ID does not exist");
        }

        std::optional<std::reference_wrapper<T>> tryGet(Id id)
        {
            auto index = extractIndex(id);

            auto& entry = vec[index.value];
            return match(
                entry,
                [](const FreeEntry&) { return std::optional<std::reference_wrapper<T>>(); },
                [id](OccupiedEntry& e) {
                    return e.first == id
                        ? std::make_optional(std::ref(e.second))
                        : std::optional<std::reference_wrapper<T>>();
                });
        }

        std::optional<std::reference_wrapper<const T>> tryGet(Id id) const
        {
            auto index = extractIndex(id);

            const auto& entry = vec[index.value];
            return match(
                entry,
                [](const FreeEntry&) { return std::optional<std::reference_wrapper<const T>>(); },
                [id](const OccupiedEntry& e) {
                    return e.first == id
                        ? std::make_optional(std::ref(e.second))
                        : std::optional<std::reference_wrapper<const T>>();
                });
        }

        iterator begin()
        {
            return iterator(firstValid(vec.begin(), vec.end()), vec.end());
        }

        iterator end()
        {
            return iterator(vec.end(), vec.end());
        }

        const_iterator begin() const
        {
            return const_iterator(firstValid(vec.begin(), vec.end()), vec.end());
        }

        const_iterator end() const
        {
            return const_iterator(vec.end(), vec.end());
        }

        iterator erase(iterator it)
        {
            remove(it->first);
            return ++it;
        }

        const_iterator find(Id id) const
        {
            auto index = extractIndex(id);
            if (index.value >= vec.size())
            {
                return end();
            }

            auto it = vec.begin() + index.value;
            return match(
                *it,
                [&](const FreeEntry&) { return end(); },
                [&](const OccupiedEntry& e) { return e.first == id ? const_iterator(it, vec.end()) : end(); });
        }

        iterator find(Id id)
        {
            auto index = extractIndex(id);
            if (index.value >= vec.size())
            {
                return end();
            }

            auto it = vec.begin() + index.value;
            return match(
                *it,
                [&](const FreeEntry&) { return end(); },
                [&](OccupiedEntry& e) { return e.first == id ? iterator(it, vec.end()) : end(); });
        }

    private:
        static Index extractIndex(Id id)
        {
            return Index(id.value >> 8u);
        }

        static std::pair<Index, Generation> parseId(Id id)
        {
            return std::make_pair(extractIndex(id), Generation(id.value & 0xFFu));
        }

        static Id makeId(Index index)
        {
            return Id(index.value << 8u);
        }
        static Id makeId(Index index, Generation generation)
        {
            return Id((index.value << 8u) | generation.value);
        }

        static Id nextGeneration(Id id)
        {
            auto [index, generation] = parseId(id);
            return makeId(index, Generation((generation.value + 1) & 0xFFu));
        }

        template <typename It>
        static It firstValid(It it, It end)
        {
            return std::find_if(it, end, [](const auto& v) { return std::holds_alternative<OccupiedEntry>(v); });
        }
    };
}

#endif
