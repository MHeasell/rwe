#include <rwe/optional_io.h>
#include <catch2/catch.hpp>
#include <rwe/geometry/BoundingBox3f.h>

namespace rwe
{
    TEST_CASE("BoundingBox3f::fromMinMax")
    {
        auto b = BoundingBox3f::fromMinMax(Vector3f(10.0f, 11.0f, 12.0f), Vector3f(11.0f, 13.0f, 15.0f));
        REQUIRE(b.center == Vector3f(10.5f, 12.0f, 13.5f));
        REQUIRE(b.extents == Vector3f(0.5f, 1.0f, 1.5f));
    }

    TEST_CASE("BoundingBox3f.distanceSquared")
    {
        SECTION("returns 0 inside the box")
        {
            auto r = BoundingBox3f(Vector3f(2.0f, 4.0f, 0.0f), Vector3f(3.0f, 5.0f, 1.0f));
            REQUIRE(r.distanceSquared(Vector3f(2.0f, 4.0f, 0.0f)) == 0.0f);
            REQUIRE(r.distanceSquared(Vector3f(4.9f, 8.9f, 0.0f)) == 0.0f);
            REQUIRE(r.distanceSquared(Vector3f(-0.9f, -0.9f, 0.0f)) == 0.0f);
        }

        SECTION("returns distance to edge of the box")
        {
            auto r = BoundingBox3f(Vector3f(2.0f, 4.0f, 0.0f), Vector3f(3.0f, 5.0f, 1.0f));
            REQUIRE(r.distanceSquared(Vector3f(-2.0f, 4.0f, 0.0f)) == 1.0f); // left
            REQUIRE(r.distanceSquared(Vector3f(6.0f, 4.0f, 0.0f)) == 1.0f); // right
            REQUIRE(r.distanceSquared(Vector3f(2.0f, -2.0f, 0.0f)) == 1.0f); // top
            REQUIRE(r.distanceSquared(Vector3f(2.0f, 10.0f, 0.0f)) == 1.0f); // bottom

            // further away
            REQUIRE(r.distanceSquared(Vector3f(-3.0f, 4.0f, 0.0f)) == 4.0f); // left
            REQUIRE(r.distanceSquared(Vector3f(7.0f, 4.0f, 0.0f)) == 4.0f); // right
            REQUIRE(r.distanceSquared(Vector3f(2.0f, -3.0f, 0.0f)) == 4.0f); // top
            REQUIRE(r.distanceSquared(Vector3f(2.0f, 11.0f, 0.0f)) == 4.0f); // bottom

            REQUIRE(r.distanceSquared(Vector3f(-2.0f, -2.0f, 0.0f)) == 2.0f); // topleft
            REQUIRE(r.distanceSquared(Vector3f(6.0f, -2.0f, 0.0f)) == 2.0f); // topright
            REQUIRE(r.distanceSquared(Vector3f(-2.0f, 10.0f, 0.0f)) == 2.0f); // bottomleft
            REQUIRE(r.distanceSquared(Vector3f(6.0f, 10.0f, 0.0f)) == 2.0f); // bottomright
        }
    }
}
