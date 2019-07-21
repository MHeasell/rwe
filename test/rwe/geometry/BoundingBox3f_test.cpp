#include <rwe/optional_io.h>
#include <catch2/catch.hpp>
#include <rwe/geometry/BoundingBox3f.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, const BoundingBox3f::RayIntersect& i)
    {
        os << "(" << std::to_string(i.enter) << ", " << std::to_string(i.exit) << ")";
        return os;
    }

    TEST_CASE("BoundingBox3f::fromMinMax")
    {
        auto b = BoundingBox3f::fromMinMax(Vector3f(10.0f, 11.0f, 12.0f), Vector3f(11.0f, 13.0f, 15.0f));
        REQUIRE(b.center == Vector3f(10.5f, 12.0f, 13.5f));
        REQUIRE(b.extents == Vector3f(0.5f, 1.0f, 1.5f));
    }

    TEST_CASE("BoundingBox3f::intersect")
    {
        SECTION("finds the point at which the ray enters the box")
        {
            BoundingBox3f box(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f));
            Ray3f ray(Vector3f(-5.0f, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f));
            auto intersect = box.intersect(ray);
            REQUIRE(intersect);
            REQUIRE(intersect->enter == Approx(4.0f));
            REQUIRE(intersect->exit == Approx(6.0f));
        }

        SECTION("hits at the corner of the box")
        {
            BoundingBox3f box(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f));
            Ray3f ray(Vector3f(-2.0f, -2.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f));
            auto intersect = box.intersect(ray);
            REQUIRE(intersect);
            REQUIRE(intersect->enter == Approx(1.0f));
            REQUIRE(intersect->exit == Approx(1.0f));
        }

        SECTION("misses just above the corner of the box")
        {
            BoundingBox3f box(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f));
            Ray3f ray(Vector3f(-2.0f, -2.0f, 0.00001f), Vector3f(1.0f, 1.0f, 1.0f));
            auto intersect = box.intersect(ray);
            REQUIRE(!intersect);
        }

        SECTION("returns a miss when the ray misses")
        {
            BoundingBox3f box(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 1.0f));
            Ray3f ray(Vector3f(-5.0f, 0.0f, 0.0f), Vector3f(1.0f, 1.0f, 0.0f));
            auto intersect = box.intersect(ray);
            REQUIRE(!intersect);
        }
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
