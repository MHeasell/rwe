#include <catch2/catch.hpp>
#include <rwe/util/Result.h>

#include <string>

namespace rwe
{
    TEST_CASE("Result")
    {
        SECTION("can be constructed")
        {
            Result<int, std::string> r(Ok(24));
            REQUIRE(r.isOk());
            REQUIRE(!r.isErr());
            REQUIRE(r.get() == 24);
            REQUIRE(*r == 24);
            REQUIRE(!!r);
        }

        SECTION("can be constructed from error")
        {
            Result<int, std::string> r(Err(std::string("whoops")));
            REQUIRE(!r.isOk());
            REQUIRE(r.isErr());
            REQUIRE(r.getErr() == "whoops");
            REQUIRE(!r);
        }
    }
}
