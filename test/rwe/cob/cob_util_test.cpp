#include <catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/cob/cob_util.h>

namespace rwe
{
    TEST_CASE("packCoords")
    {
        SECTION("packs trivial positive values")
        {
            float a = 5; // 0x0005
            float b = 3; // 0x0003
            REQUIRE(packCoords(a, b) == 0x00050003);
        }

        SECTION("packs negative values")
        {
            float a = -5; // 0xfffb
            float b = -3; // 0xfffd
            REQUIRE(packCoords(a, b) == 0xfffbfffd);
        }

        SECTION("packs very low negative values")
        {
            float a = -28675; // 0x8ffd
            float b = -24412; // 0xa0a4
            REQUIRE(packCoords(a, b) == 0x8ffda0a4);
        }
    }

    TEST_CASE("unpackCoords")
    {
        rc::prop("pack followed by unpack is the identity", [](int16_t a, int16_t b) {
            // all int16 values can be represented exactly by floats
            auto fA = static_cast<float>(a);
            auto fB = static_cast<float>(b);

            auto result = unpackCoords(packCoords(fA, fB));

            RC_ASSERT(result.first == fA);
            RC_ASSERT(result.second == fB);
        });
    }

    TEST_CASE("cobAtan")
    {
        REQUIRE(cobAtan(0, 0) == 0);
        REQUIRE(cobAtan(1, 1) == 8192);
        REQUIRE(cobAtan(1, 0) == 16384);
        REQUIRE(cobAtan(0, -1) == 32768);
        REQUIRE(cobAtan(-1, 0) == 49152);
        REQUIRE(cobAtan(0, 1) == 0);
    }
}
