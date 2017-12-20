#include <catch.hpp>
#include <rwe/MinHeap.h>

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
                    [](const auto& p)
                    { return p.first; },
                    [](const auto& a, const auto& b)
                    { return a.second < b.second; });

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
                    [](const auto& p)
                    { return p.first; },
                    [](const auto& a, const auto& b)
                    { return a.second < b.second; });

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
    }
}
