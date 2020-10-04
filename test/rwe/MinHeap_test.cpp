#include <iostream>

namespace std
{
    std::ostream& operator<<(std::ostream& os, const std::pair<int, int>& p)
    {
        os << "(" << p.first << ", " << p.second << ")";
        return os;
    }
}

#include <boost/functional/hash.hpp>
#include <catch2/catch.hpp>
#include <optional>
#include <rapidcheck/boost.h>
#include <rapidcheck/catch.h>
#include <rwe/MinHeap.h>
#include <rwe/grid/Point.h>
#include <rwe/optional_io.h>
#include <rwe/rc_gen_optional.h>

namespace std
{
    template <typename A, typename B>
    struct hash<std::pair<A, B>>
    {
        std::size_t operator()(const std::pair<A, B>& f) const noexcept
        {
            return boost::hash<std::pair<A, B>>()(f);
        }
    };
}

namespace rwe
{
    TEST_CASE("MinHeap")
    {
        SECTION("starts empty")
        {
            MinHeap<int> heap;
            REQUIRE(heap.empty());
        }

        SECTION("is not empty after insertion")
        {
            MinHeap<int> heap;
            heap.pushOrDecrease(1);
            REQUIRE(!heap.empty());
        }

        SECTION("emits elements in priority order")
        {
            MinHeap<int> heap;
            heap.pushOrDecrease(1);
            heap.pushOrDecrease(5);
            heap.pushOrDecrease(8);
            heap.pushOrDecrease(2);
            heap.pushOrDecrease(3);
            heap.pushOrDecrease(4);
            heap.pushOrDecrease(2);
            heap.pushOrDecrease(9);

            REQUIRE(heap.top() == 1);
            heap.pop();
            REQUIRE(heap.top() == 2);
            heap.pop();
            REQUIRE(heap.top() == 3);
            heap.pop();
            REQUIRE(heap.top() == 4);
            heap.pop();
            REQUIRE(heap.top() == 5);
            heap.pop();
            REQUIRE(heap.top() == 8);
            heap.pop();
            REQUIRE(heap.top() == 9);
            heap.pop();
            REQUIRE(heap.empty());
        }

        SECTION(".insertOrDecrease")
        {
            SECTION("inserts elements in priority order")
            {
                MinHeap<int> heap;
                heap.pushOrDecrease(1);
                heap.pushOrDecrease(3);
                heap.pushOrDecrease(2);

                REQUIRE(heap.top() == 1);
                heap.pop();
                REQUIRE(heap.top() == 2);
                heap.pop();
                REQUIRE(heap.top() == 3);
                heap.pop();
                REQUIRE(heap.empty());
            }

            SECTION("decreases the key of an element")
            {
                auto heap = createMinHeap<int, std::pair<int, double>>(
                    [](const auto& p) { return p.first; },
                    [](const auto& a, const auto& b) { return a.second < b.second; });

                heap.pushOrDecrease({1, 1.0});
                heap.pushOrDecrease({2, 4.0});
                heap.pushOrDecrease({3, 3.0});
                heap.pushOrDecrease({4, 6.0});
                heap.pushOrDecrease({5, 7.0});
                heap.pushOrDecrease({2, 2.0});

                {
                    std::pair<int, double> p(1, 1.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(2, 2.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(3, 3.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(4, 6.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(5, 7.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                REQUIRE(heap.empty());
            }

            SECTION("doesn't decrease the key when the existing key is better")
            {
                auto heap = createMinHeap<int, std::pair<int, double>>(
                    [](const auto& p) { return p.first; },
                    [](const auto& a, const auto& b) { return a.second < b.second; });

                heap.pushOrDecrease({1, 1.0});
                heap.pushOrDecrease({2, 4.0});
                heap.pushOrDecrease({3, 3.0});
                heap.pushOrDecrease({4, 6.0});
                heap.pushOrDecrease({5, 7.0});
                heap.pushOrDecrease({2, 6.0});

                {
                    std::pair<int, double> p(1, 1.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(3, 3.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(2, 4.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(4, 6.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                {
                    std::pair<int, double> p(5, 7.0);
                    REQUIRE(heap.top() == p);
                }
                heap.pop();
                REQUIRE(heap.empty());
            }
        }

        SECTION("works with these values found by RapidCheck")
        {
            auto selectKey = [](const std::pair<int, int>& p) { return p.first; };
            auto lessThan = [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second < b.second; };
            auto heap = createMinHeap<int, std::pair<int, int>>(selectKey, lessThan);

            // RapidCheck found that the heap would fail
            // to correctly decrease the priority of the '-2'
            // after inserting the '0'
            // due to bad bookkeeping of the indexMap inside the heap.
            heap.pushOrDecrease({-2, 0});
            heap.pushOrDecrease({0, -1});
            heap.pushOrDecrease({-2, -1});

            {
                std::pair<int, int> p(0, -1);
                REQUIRE(heap.top() == p);
            }
            heap.pop();
            {
                std::pair<int, int> p(-2, -1);
                REQUIRE(heap.top() == p);
            }
            heap.pop();

            REQUIRE(heap.empty());
        }

        SECTION("works with these values found by RapidCheck, round 2")
        {
            auto selectKey = [](const std::pair<int, int>& p) { return p.first; };
            auto lessThan = [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second < b.second; };
            auto heap = createMinHeap<int, std::pair<int, int>>(selectKey, lessThan);
            REQUIRE(heap.isNotCorrupt());

            heap.pushOrDecrease({0, 0});
            REQUIRE(heap.isNotCorrupt());

            heap.pushOrDecrease({1, 0});
            REQUIRE(heap.isNotCorrupt());

            heap.pop();
            REQUIRE(heap.isNotCorrupt());
        }
    }

    TEST_CASE("MinHeap RapidCheck")
    {
        rc::prop("always emits correctly sorted values", [](const std::vector<std::pair<int, int>>& inputNumbers) {
            auto selectKey = [](const std::pair<int, int>& p) { return p.first; };
            auto lessThan = [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second < b.second; };

            // push all the numbers onto the heap,
            // then pull them out into a sorted, unique vector.
            auto heap = createMinHeap<int, std::pair<int, int>>(selectKey, lessThan);

            for (const auto& elem : inputNumbers)
            {
                heap.pushOrDecrease(elem);
            }

            std::vector<std::pair<int, int>> outputNumbers;
            while (!heap.empty())
            {
                outputNumbers.push_back(heap.top());
                heap.pop();
            }

            // create a sorted, unique vector to check against
            // by manually sorting, then removing duplicate keys.
            // (Note that this list could reasonably be not be identical to the output
            // because ordering of elements with the same priority
            // is not strictly defined, so we will have to be a bit cleverer
            // when we do comparisons to work around this.)
            auto sortedNumbers = inputNumbers;

            std::sort(sortedNumbers.begin(), sortedNumbers.end(), lessThan);
            std::unordered_set<int> seenKeys;

            std::vector<std::pair<int, int>> expectedNumbers;
            for (const auto& item : sortedNumbers)
            {
                if (seenKeys.find(selectKey(item)) == seenKeys.end())
                {
                    expectedNumbers.push_back(item);
                    seenKeys.insert(selectKey(item));
                }
            }

            // verify the lists are the same size
            RC_ASSERT(outputNumbers.size() == expectedNumbers.size());

            // verify the lists contain the same frequency of elements
            std::unordered_map<std::pair<int, int>, int> outputFreqs;
            for (const auto& item : outputNumbers)
            {
                outputFreqs[item]++;
            }

            std::unordered_map<std::pair<int, int>, int> expectedFreqs;
            for (const auto& item : expectedNumbers)
            {
                expectedFreqs[item]++;
            }

            RC_ASSERT(outputFreqs == expectedFreqs);

            // verify that the list is sorted
            RC_ASSERT(std::is_sorted(outputNumbers.begin(), outputNumbers.end(), lessThan));
        });
    }

    TEST_CASE("MinHeap RapidCheck 2")
    {
        rc::prop("never becomes internally corrupted", [](const std::vector<std::optional<std::pair<int, int>>>& inputs) {
            auto selectKey = [](const std::pair<int, int>& p) { return p.first; };
            auto lessThan = [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second < b.second; };

            auto heap = createMinHeap<int, std::pair<int, int>>(selectKey, lessThan);

            std::vector<std::pair<int, int>> dumbQueue;

            for (const auto& elem : inputs)
            {
                if (elem)
                {
                    // insert onto the heap
                    heap.pushOrDecrease(*elem);
                }
                else
                {
                    // pop the heap
                    if (!heap.empty())
                    {
                        heap.pop();
                    }
                }

                RC_ASSERT(heap.isNotCorrupt());
            }
        });
    }
}
