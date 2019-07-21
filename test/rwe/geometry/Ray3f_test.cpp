#include <catch2/catch.hpp>
#include <rwe/geometry/Ray3f.h>

namespace rwe
{
    TEST_CASE("Ray3f::pointAt")
    {
        SECTION("Gets the point after travelling for some time")
        {
            Ray3f r(Vector3f(3.0f, 10.0f, 4.0f), Vector3f(0.0f, -2.0f, 0.0f));
            auto i = r.pointAt(3.0f);
            REQUIRE(i.x == Approx(3.0f));
            REQUIRE(i.y == Approx(4.0f));
            REQUIRE(i.z == Approx(4.0f));
        }
    }

    TEST_CASE("Ray3f::isLessFar")
    {
        SECTION("returns true when point a is the least far along")
        {
            Ray3f r(Vector3f(3.0f, 10.0f, 4.0f), Vector3f(0.0f, -2.0f, 0.0f));
            REQUIRE(r.isLessFar(r.pointAt(1.0f), r.pointAt(2.0f)));
            REQUIRE(!r.isLessFar(r.pointAt(3.0f), r.pointAt(2.0f)));
            REQUIRE(r.isLessFar(r.pointAt(-5.0f), r.pointAt(-4.0f)));
            REQUIRE(!r.isLessFar(r.pointAt(-5.0f), r.pointAt(-6.0f)));
        }
    }
}
