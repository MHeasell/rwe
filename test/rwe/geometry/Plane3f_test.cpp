#include <boost/optional/optional_io.hpp>
#include <catch.hpp>
#include <rwe/math/Vector3f.h>
#include <rwe/geometry/Ray3f.h>
#include <rwe/geometry/Plane3f.h>

namespace rwe
{
    TEST_CASE("Plane3f::intersect")
    {
        SECTION("Gets the time of intersection with a ray")
        {
            Ray3f r(Vector3f(3.0f, 10.0f, 4.0f), Vector3f(0.0f, -2.0f, 0.0f));
            Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = p.intersect(r);
            REQUIRE(intersect);
            REQUIRE(*intersect == 5.0f);
        }

        SECTION("Returns a miss when the ray is parallel in front of the plane")
        {
            Ray3f r(Vector3f(1.0f, 1.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f));
            Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = p.intersect(r);
            REQUIRE(!intersect);
        }

        SECTION("Returns a miss when the ray is parallel behind the plane")
        {
            Ray3f r(Vector3f(1.0f, -1.0f, 1.0f), Vector3f(1.0f, 0.0f, 0.0f));
            Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = p.intersect(r);
            REQUIRE(!intersect);
        }
    }

    TEST_CASE("Plane3f::isInFront")
    {
        SECTION("returns true when point is on the side the normal points to")
        {
            {
                Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
                Vector3f v(3.0f, 4.0f, 5.0f);
                REQUIRE(p.isInFront(v));
            }
            {
                Plane3f p(Vector3f(3.0f, 4.0f, 5.0f), Vector3f(0.0f, 1.0f, 0.0f));
                Vector3f v(-1.0f, 5.0f, 17.0f);
                REQUIRE(p.isInFront(v));
            }
        }

        SECTION("returns false when point is on the other side")
        {
            {
                Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
                Vector3f v(3.0f, -4.0f, 5.0f);
                REQUIRE(!p.isInFront(v));
            }

            {
                Plane3f p(Vector3f(3.0f, 4.0f, 5.0f), Vector3f(0.0f, 1.0f, 0.0f));
                Vector3f v(2.0f, 3.0f, 11.0f);
                REQUIRE(!p.isInFront(v));
            }
        }

        SECTION("returns false when the point lies on the plane")
        {
            Plane3f p(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f));
            Vector3f v(3.0f, 0.0f, 5.0f);
            REQUIRE(!p.isInFront(v));
        }
    }

    TEST_CASE("Plane3f::fromPoints")
    {
        SECTION("creates a plane from 3 points in anti-clockwise winding")
        {
            Vector3f a(0.0f, 0.0f, 0.0f);
            Vector3f b(1.0f, 0.0f, 0.0f);
            Vector3f c(1.0f, 1.0f, 0.0f);

            Plane3f p = Plane3f::fromPoints(a, b, c);

            // not strictly required by implementation
            // (could use b or c as the point)
            // but no better way to verify at the moment
            REQUIRE(p.point.x == Approx(0.0f));
            REQUIRE(p.point.y == Approx(0.0f));
            REQUIRE(p.point.z == Approx(0.0f));

            REQUIRE(p.normal.x == Approx(0.0f));
            REQUIRE(p.normal.y == Approx(0.0f));
            REQUIRE(p.normal.z == Approx(1.0f));
        }
    }
}
