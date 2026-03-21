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

    TEST_CASE("computeCrc")
    {
        SECTION("empty input")
        {
            REQUIRE(computeCrc("", 0) == 0x00000000u);
        }

        SECTION("known CRC32 values")
        {
            // CRC32 of "123456789" is 0xCBF43926
            const char* input = "123456789";
            REQUIRE(computeCrc(input, 9) == 0xCBF43926u);
        }

        SECTION("single byte")
        {
            // CRC32 of "a" is 0xE8B7BE43
            REQUIRE(computeCrc("a", 1) == 0xE8B7BE43u);
        }
    }
}
