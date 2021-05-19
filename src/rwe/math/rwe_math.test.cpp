#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

namespace rwe
{
    TEST_CASE("wrap")
    {
        SECTION("int overload")
        {
            SECTION("two-arg")
            {
                REQUIRE(wrap(-6, 5) == 4);
                REQUIRE(wrap(-5, 5) == 0);

                REQUIRE(wrap(-2, 5) == 3);
                REQUIRE(wrap(-1, 5) == 4);

                REQUIRE(wrap(0, 5) == 0);
                REQUIRE(wrap(1, 5) == 1);
                REQUIRE(wrap(2, 5) == 2);
                REQUIRE(wrap(3, 5) == 3);
                REQUIRE(wrap(4, 5) == 4);
                REQUIRE(wrap(5, 5) == 0);
                REQUIRE(wrap(6, 5) == 1);

                REQUIRE(wrap(10, 5) == 0);
                REQUIRE(wrap(11, 5) == 1);
            }

            SECTION("three-arg")
            {
                REQUIRE(wrap(1, 3, 0) == 2);
                REQUIRE(wrap(1, 3, 1) == 1);
                REQUIRE(wrap(1, 3, 2) == 2);
                REQUIRE(wrap(1, 3, 3) == 1);

                REQUIRE(wrap(-2, 5, -1) == -1);
                REQUIRE(wrap(-2, 5, -3) == 4);
            }
        }

        SECTION("float overload")
        {
            SECTION("doesn't touch the input if it's in range")
            {
                REQUIRE(wrap(-Pif, Pif, 0.0f) == 0.0f);
            }
            rc::prop("two-arg", []() {
                auto value = *rc::gen::inRange(-100'000, 100'000);
                auto max = *rc::gen::inRange(1, 100'000);
                RC_ASSERT(wrap(value, max) == wrap(static_cast<float>(value), static_cast<float>(max)));
            });
            rc::prop("three-arg", []() {
                auto value = *rc::gen::inRange(-100'000, 100'000);
                auto a = *rc::gen::inRange(-100'000, 100'000);
                auto b = *rc::gen::inRange(-100'000, 100'000);
                RC_PRE(a != b);
                auto minmax = std::minmax(a, b);
                RC_ASSERT(wrap(minmax.first, minmax.second, value) == wrap(static_cast<float>(minmax.first), static_cast<float>(minmax.second), static_cast<float>(value)));
            });
        }
    }

    TEST_CASE("roundUpToPowerOfTwo")
    {
        SECTION("rounds up values to the next power of two")
        {
            // note output = 0 for input == 0 and input that would round too high to fit in this data type

            // edges of validity: 1, 1 << (sizeof(T)*8) - 1

            unsigned short input = 0;

            for (int i = 0; i < sizeof(i) * 8; i++)
            {
                const int power = 1 << i;
                REQUIRE(roundUpToPowerOfTwo(power) == power);
                if (i < (sizeof(i) * 8) - 2)
                {
                    const int up = power + 1;
                    REQUIRE(roundUpToPowerOfTwo(up) == power << 1);
                
                }
                if (i > 1)
                {
                    const int down = power - 1;
                    REQUIRE(roundUpToPowerOfTwo(down) == power >> 1);
                }
            }
            REQUIRE(roundUpToPowerOfTwo(1) == 1);
            REQUIRE(roundUpToPowerOfTwo(2) == 2);
            REQUIRE(roundUpToPowerOfTwo(3) == 4);
            REQUIRE(roundUpToPowerOfTwo(4) == 4);
            REQUIRE(roundUpToPowerOfTwo(64) == 64);
            REQUIRE(roundUpToPowerOfTwo(65) == 128);

            REQUIRE(roundUpToPowerOfTwo(2'000'000'000) == 2'147'483'648);
        }
    }

    TEST_CASE("sameSign")
    {
        REQUIRE(sameSign(1.0f, 2.0f));
        REQUIRE(sameSign(-1.0f, -2.0f));

        REQUIRE(!sameSign(1.0f, -2.0f));

        REQUIRE(sameSign(0.0f, 1.0f));
        REQUIRE(sameSign(0.0f, -1.0f));
    }
}
