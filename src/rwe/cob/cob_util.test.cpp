#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/cob/cob_util.h>

namespace rwe
{
    TEST_CASE("packCoords")
    {
        SECTION("packs trivial positive values")
        {
            auto a = 5_ss; // 0x0005
            auto b = 3_ss; // 0x0003
            REQUIRE(packCoords(a, b) == 0x00050003);
        }

        SECTION("packs negative values")
        {
            auto a = -5_ss; // 0xfffb
            auto b = -3_ss; // 0xfffd
            REQUIRE(packCoords(a, b) == 0xfffbfffd);
        }

        SECTION("packs very low negative values")
        {
            auto a = -28675_ss; // 0x8ffd
            auto b = -24412_ss; // 0xa0a4
            REQUIRE(packCoords(a, b) == 0x8ffda0a4);
        }
    }

    TEST_CASE("unpackCoords")
    {
        rc::prop("pack followed by unpack is the identity", [](int16_t a, int16_t b) {
            // all int16 values can be represented exactly by floats
            auto fA = SimScalar(a);
            auto fB = SimScalar(b);

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
