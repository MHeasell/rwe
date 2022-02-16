#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/cob/cob_util.h>

namespace rwe
{
    TEST_CASE("cobPackCoords")
    {
        SECTION("packs trivial positive values")
        {
            auto a = CobPosition::fromInt(5); // 0x0005
            auto b = CobPosition::fromInt(3); // 0x0003
            REQUIRE(cobPackCoords(a, b) == 0x00050003);
        }

        SECTION("packs negative values")
        {
            auto a = CobPosition::fromInt(-5); // 0xfffb
            auto b = CobPosition::fromInt(-3); // 0xfffd
            REQUIRE(cobPackCoords(a, b) == 0xfffbfffd);
        }

        SECTION("packs very low negative values")
        {
            auto a = CobPosition::fromInt(-28675); // 0x8ffd
            auto b = CobPosition::fromInt(-24412); // 0xa0a4
            REQUIRE(cobPackCoords(a, b) == 0x8ffda0a4);
        }
    }

    TEST_CASE("cobUnpackCoords")
    {
        rc::prop("pack followed by unpack is the identity", [](int16_t a, int16_t b) {
            auto result = cobUnpackCoords(cobPackCoords(CobPosition::fromInt(a), CobPosition::fromInt(b)));
            RC_ASSERT(result.first == CobPosition::fromInt(a));
            RC_ASSERT(result.second == CobPosition::fromInt(b));
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
