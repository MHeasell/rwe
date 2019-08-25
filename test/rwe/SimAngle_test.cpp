#include <catch2/catch.hpp>
#include <rapidcheck/catch.h>
#include <rwe/OpaqueId_io.h>
#include <rwe/SimAngle.h>
#include <rwe/util.h>

namespace rc
{
    template<>
    struct Arbitrary<rwe::SimAngle>
    {
        static Gen<rwe::SimAngle> arbitrary()
        {
            return gen::build<rwe::SimAngle>(
                gen::set(&rwe::SimAngle::value, gen::arbitrary<rwe::SimAngle::ValueType>()));
        }
    };
}

namespace rwe
{
    template <typename A, typename B>
    std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& v)
    {
        os << "{" << v.first << ", " << v.second << "}";
        return os;
    }

    TEST_CASE("negation")
    {
        SECTION("works")
        {
            REQUIRE(-SimAngle(0) == SimAngle(0));
            REQUIRE(-SimAngle(65535) == SimAngle(1));
        }
    }

    TEST_CASE("toRadians")
    {
        SECTION("converts SimAngle to radians")
        {
            REQUIRE(toRadians(SimAngle(0)) == RadiansAngle(0.0f));
            REQUIRE(toRadians(SimAngle(16384)).value == Approx(Pif / 2.0f));
            REQUIRE(toRadians(SimAngle(32768)).value == Approx(-Pif));
            REQUIRE(toRadians(SimAngle(49152)).value == Approx(-Pif / 2.0f));
        }

        rc::prop("fromRadians inverts toRadians", [](SimAngle a) {
          auto f = toRadians(a);
          RC_LOG() << "f: " << f << std::endl;
          auto t2 = fromRadians(f);
          RC_ASSERT(t2.value == a.value);
        });
    }

    TEST_CASE("angleBetween")
    {
        SECTION("returns smallest angle between two angles")
        {
            REQUIRE(angleBetween(SimAngle(100), SimAngle(150)) == SimAngle(50));
            REQUIRE(angleBetween(SimAngle(100), SimAngle(300)) == SimAngle(200));

            REQUIRE(angleBetween(SimAngle(1), SimAngle(0)) == SimAngle(1));
        }
        SECTION("works across the discontinuity")
        {
            REQUIRE(angleBetween(SimAngle(65530), SimAngle(5)) == SimAngle(11));
        }

        rc::prop("arg order doesn't matter", [](SimAngle a, SimAngle b) {
            RC_ASSERT(angleBetween(a, b) == angleBetween(b, a));
        });

        rc::prop("result never more than half a turn", [](SimAngle a, SimAngle b) {
          RC_ASSERT(angleBetween(a, b).value <= HalfTurn.value);
        });
    }

    TEST_CASE("angleBetweenWithDirection")
    {
        SECTION("returns smallest angle between two angles, with direction")
        {
            REQUIRE(angleBetweenWithDirection(SimAngle(100), SimAngle(150)) == std::make_pair(true, SimAngle(50)));
            REQUIRE(angleBetweenWithDirection(SimAngle(100), SimAngle(300)) == std::make_pair(true, SimAngle(200)));
        }
        SECTION("works across the discontinuity")
        {
            REQUIRE(angleBetweenWithDirection(SimAngle(65530), SimAngle(5)) == std::make_pair(true, SimAngle(11)));
        }

        rc::prop("emits same values as angleBetween", [](SimAngle a, SimAngle b) {
            RC_ASSERT(angleBetweenWithDirection(a, b).second == angleBetween(a, b));
        });
        rc::prop("same angles always zero", [](SimAngle a) {
            RC_ASSERT(angleBetweenWithDirection(a, a) == std::make_pair(true, SimAngle(0)));
        });
        rc::prop("arg order flips direction", [](SimAngle a, SimAngle b) {
            RC_PRE(a != b);
            RC_ASSERT(angleBetweenWithDirection(a, b).first != angleBetweenWithDirection(b, a).first);
        });
    }

    TEST_CASE("turnTowards")
    {
        SECTION("when max turn greater than delta, returns destination")
        {
            REQUIRE(turnTowards(SimAngle(100), SimAngle(200), SimAngle(150)) == SimAngle(200));
            REQUIRE(turnTowards(SimAngle(100), SimAngle(200), SimAngle(100)) == SimAngle(200));
            REQUIRE(turnTowards(SimAngle(200), SimAngle(100), SimAngle(150)) == SimAngle(100));

        }
        SECTION("when max turn less than delta, advances towards destination by max turn")
        {
            REQUIRE(turnTowards(SimAngle(100), SimAngle(200), SimAngle(25)) == SimAngle(125));
            REQUIRE(turnTowards(SimAngle(200), SimAngle(100), SimAngle(25)) == SimAngle(175));
        }
        SECTION("works across the discontinuity")
        {
            REQUIRE(turnTowards(SimAngle(65530), SimAngle(20), SimAngle(9)) == SimAngle(3));
            REQUIRE(turnTowards(SimAngle(20), SimAngle(65530), SimAngle(9)) == SimAngle(11));
        }
    }
}
