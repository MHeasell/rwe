#include <catch.hpp>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    TEST_CASE("convertScreenToClipSpace")
    {
        SECTION("works in a simple case")
        {
            auto v = convertScreenToClipSpace(8, 6, Point(2, 2));
            REQUIRE(v.x == -0.5f);
            REQUIRE(v.y == Approx(0.33333333));
        }
    }

    TEST_CASE("wrap")
    {
        SECTION("wraps floats")
        {
            REQUIRE(wrap(1.0f, 3.0f, 2.0f) == 2.0f);
            REQUIRE(wrap(1.0f, 3.0f, 3.5f) == 1.5f);
            REQUIRE(wrap(1.0f, 3.0f, 0.5f) == 2.5f);

            REQUIRE(wrap(-2.0f, 5.0f, -1.0f) == -1.0f);
            REQUIRE(wrap(-2.0f, 5.0f, -3.0f) == 4.0f);
        }

        SECTION("wraps ints")
        {
            REQUIRE(wrap(1, 3, 2) == 2);
            REQUIRE(wrap(1, 3, 3) == 1);
            REQUIRE(wrap(1, 3, 0) == 2);

            REQUIRE(wrap(-2, 5, -1) == -1);
            REQUIRE(wrap(-2, 5, -3) == 4);
        }
    }

    TEST_CASE("roundUpToPowerOfTwo")
    {
        SECTION("rounds up values to the next power of two")
        {
            REQUIRE(roundUpToPowerOfTwo(1) == 1);
            REQUIRE(roundUpToPowerOfTwo(2) == 2);
            REQUIRE(roundUpToPowerOfTwo(3) == 4);
            REQUIRE(roundUpToPowerOfTwo(4) == 4);
            REQUIRE(roundUpToPowerOfTwo(64) == 64);
            REQUIRE(roundUpToPowerOfTwo(65) == 128);

            REQUIRE(roundUpToPowerOfTwo(2'000'000'000) == 2'147'483'648);
        }
    }
}
