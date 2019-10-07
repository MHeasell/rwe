#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/network_util.h>

namespace rwe
{
    TEST_CASE("ema")
    {
        SECTION("combines value with average according to weight")
        {
            REQUIRE(ema(1.0f, 8.0f, 0.5f) == 4.5f);
            REQUIRE(ema(1.0f, 8.0f, 0.25f) == 6.25f);
        }
    }

    TEST_CASE("readInt")
    {
        rc::prop("readInt inverts writeInt", [](unsigned int i) {
            std::array<char, 4> arr;
            writeInt(arr.data(), i);
            auto result = readInt(arr.data());
            RC_ASSERT(result == i);
        });
    }
}
