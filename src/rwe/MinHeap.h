#ifndef RWE_MINHEAP_H
#define RWE_MINHEAP_H

#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <vector>

namespace rwe
{
    template <typename T>
    class Identity
    {
    public:
        const T& operator()(const T& t) const { return t; }
        T& operator()(T& t) const { return t; }
    };

    template <typename K, typename V = K, typename KeySelector = Identity<V>, typename LessThan = std::less<V>>
    class MinHeap
    {
    private:
        std::vector<V> heap;
        std::unordered_map<K, std::size_t> indexMap;

        KeySelector keySelector;
        LessThan lessThan;

    public:
        MinHeap() = default;
        explicit MinHeap(const KeySelector& selector) : keySelector(selector) {}
        MinHeap(const KeySelector& selector, const LessThan& lessThan) : keySelector(selector), lessThan(lessThan) {}

        const V& top() const
        {
            return heap.front();
        }

        bool empty() const
        {
            return heap.empty();
        }

        void pop()
        {
            auto firstElement = heap.front();
            auto lastElement = heap.back();
            heap.pop_back();
            indexMap.erase(keySelector(lastElement));

            if (heap.size() > 0)
            {
                indexMap.erase(keySelector(firstElement));
                siftDown(0, lastElement);
            }
        }

        bool pushOrDecrease(const V& item)
        {
            auto existingIt = indexMap.find(keySelector(item));
            if (existingIt == indexMap.end())
            {
                push(item);
                return true;
            }

            const auto& existingItem = heap[existingIt->second];
            assert(keySelector(existingItem) == keySelector(item));
            if (!lessThan(item, existingItem))
            {
                return false;
            }

            siftUp(existingIt->second, item);

            return true;
        }

        bool isNotCorrupt()
        {
            if (heap.size() != indexMap.size())
            {
                return false;
            }

            // really tough, performance crippling debugging assertion
            for (const auto& indexItem : indexMap)
            {
                if (indexItem.second >= heap.size())
                {
                    return false;
                }

                const auto& heapItem = heap[indexItem.second];
                if (keySelector(heapItem) != indexItem.first)
                {
                    return false;
                }
            }

            return true;
        }

    private:
        void push(const V& e)
        {
            heap.resize(heap.size() + 1);
            siftUp(heap.size() - 1, e);
        }

        void siftUp(std::size_t position, const V& element)
        {
            while (position > 0)
            {
                auto parentPosition = (position - 1) / 2;
                const auto& parentElement = heap[parentPosition];
                if (!lessThan(element, parentElement))
                {
                    break;
                }

                heap[position] = parentElement;
                indexMap[keySelector(parentElement)] = position;
                position = parentPosition;
            }

            heap[position] = element;
            indexMap[keySelector(element)] = position;
        }

        void siftDown(std::size_t position, const V& element)
        {
            auto firstLeafPosition = heap.size() / 2;
            while (position < firstLeafPosition) // while non-leaf
            {
                auto smallestChildPosition = (position * 2) + 1;
                const auto* smallestChild = &heap[smallestChildPosition];
                auto rightChildPosition = (position * 2) + 2;
                if (rightChildPosition < heap.size())
                {
                    const auto* rightChild = &heap[rightChildPosition];
                    if (lessThan(*rightChild, *smallestChild))
                    {
                        smallestChildPosition = rightChildPosition;
                        smallestChild = rightChild;
                    }
                }

                if (lessThan(element, *smallestChild))
                {
                    break;
                }

                heap[position] = *smallestChild;
                indexMap[keySelector(*smallestChild)] = position;
                position = smallestChildPosition;
            }

            heap[position] = element;
            indexMap[keySelector(element)] = position;
        }
    };

    template <typename K, typename V, typename KeySelector, typename LessThan>
    MinHeap<K, V, KeySelector, LessThan> createMinHeap(const KeySelector& keySelector, const LessThan& lessThan)
    {
        return MinHeap<K, V, KeySelector, LessThan>(keySelector, lessThan);
    };
}

#endif
