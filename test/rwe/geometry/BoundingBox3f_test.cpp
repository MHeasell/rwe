#include <boost/optional/optional_io.hpp>
#include <catch.hpp>
#include <rwe/geometry/BoundingBox3f.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, const BoundingBox3f::RayIntersect& i)
    {
        os << "(" << std::to_string(i.enter) << ", " << std::to_string(i.exit) << ")";
        return os;
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
}
