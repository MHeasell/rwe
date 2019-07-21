#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/util.h>

namespace Catch
{
    namespace Detail
    {
        std::ostream& operator<<(std::ostream& os, const Approx& a)
        {
            os << a.toString();
            return os;
        }
    }
}

namespace rwe
{
    TEST_CASE("util")
    {
        SECTION("toRadians")
        {
            REQUIRE(toRadians(TaAngle(0)).value == Approx(0.0f));
            REQUIRE(toRadians(TaAngle(8192)).value == Approx(Pif / 4.0f));
            REQUIRE(toRadians(TaAngle(16384)).value == Approx(Pif / 2.0f));
            REQUIRE(toRadians(TaAngle(32768)).value == Approx(-Pif));
            REQUIRE(toRadians(TaAngle(49152)).value == Approx(-Pif / 2.0f));
        }

        rc::prop("toTaAngle inverts toRadians", [](uint16_t a) {
            TaAngle t(a);
            auto t2 = toTaAngle(toRadians(t));
            RC_ASSERT(t2.value == t.value);
        });
    }
}
