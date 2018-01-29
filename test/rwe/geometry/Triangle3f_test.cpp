#include <boost/optional/optional_io.hpp>
#include <catch.hpp>
#include <rwe/geometry/Triangle3f.h>

namespace rwe
{
    TEST_CASE("Triangle3f::toBarycentric")
    {
        SECTION("barycentric conversion test 1")
        {
            Vector3f p(1.0f, 0.0f, 0.0f);
            Triangle3f tri(
                Vector3f(0.0f, 0.0f, 0.0f),
                Vector3f(2.0f, 0.0f, 0.0f),
                Vector3f(0.0f, 2.0f, 0.0f));
            Vector3f bary = tri.toBarycentric(p);

            REQUIRE(bary.x == Approx(0.5f));
            REQUIRE(bary.y == Approx(0.5f));
            REQUIRE(bary.z == Approx(0.0f));
        }

        SECTION("barycentric conversion test 2")
        {
            Vector3f p(0.5f, 0.5f, 0.0f);
            Triangle3f tri(
                Vector3f(0.0f, 0.0f, 0.0f),
                Vector3f(2.0f, 0.0f, 0.0f),
                Vector3f(0.0f, 2.0f, 0.0f));
            Vector3f bary = tri.toBarycentric(p);

            REQUIRE(bary.x == Approx(0.5f));
            REQUIRE(bary.y == Approx(0.25f));
            REQUIRE(bary.z == Approx(0.25f));
        }

        SECTION("barycentric conversion test 3")
        {
            Vector3f p(-5.0f, 0.5f, 5.0f);
            Triangle3f tri(
                Vector3f(-0.5f, 0.5f, -0.5f),
                Vector3f(0.5f, 0.5f, -0.5f),
                Vector3f(0.5f, 0.5f, 0.5f));
            Vector3f bary = tri.toBarycentric(p);

            REQUIRE(bary.x == Approx(5.5f));
            REQUIRE(bary.y == Approx(-10.0f));
            REQUIRE(bary.z == Approx(5.5f));
        }
    }

    TEST_CASE("Triangle3f::toCartesian")
    {
        SECTION("test 1")
        {
            Vector3f p(0.5f, 0.5f, 0.0f);
            Triangle3f tri(
                Vector3f(0.0f, 0.0f, 0.0f),
                Vector3f(2.0f, 0.0f, 0.0f),
                Vector3f(0.0f, 2.0f, 0.0f));
            Vector3f cart = tri.toCartesian(p);

            REQUIRE(cart.x == Approx(1.0f));
            REQUIRE(cart.y == Approx(0.0f));
            REQUIRE(cart.z == Approx(0.0f));
        }

        SECTION("test 2")
        {
            Vector3f p(0.5f, 0.25f, 0.25f);
            Triangle3f tri(
                Vector3f(0.0f, 0.0f, 0.0f),
                Vector3f(2.0f, 0.0f, 0.0f),
                Vector3f(0.0f, 2.0f, 0.0f));
            Vector3f cart = tri.toCartesian(p);

            REQUIRE(cart.x == Approx(0.5f));
            REQUIRE(cart.y == Approx(0.5f));
            REQUIRE(cart.z == Approx(0.0f));
        }

        SECTION("test 3")
        {
            Vector3f p(5.5f, -10.0f, 5.5f);
            Triangle3f tri(
                Vector3f(-0.5f, 0.5f, -0.5f),
                Vector3f(0.5f, 0.5f, -0.5f),
                Vector3f(0.5f, 0.5f, 0.5f));
            Vector3f cart = tri.toCartesian(p);

            REQUIRE(cart.x == Approx(-5.0f));
            REQUIRE(cart.y == Approx(0.5f));
            REQUIRE(cart.z == Approx(5.0f));
        }
    }

    TEST_CASE("Triangle3f::intersect")
    {
        SECTION("returns the distance when the ray hits")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            Ray3f r(
                Vector3f(0.0f, 0.0f, 10.0f),
                Vector3f(0.0f, 0.0f, -1.0f));
            auto intersect = tri.intersect(r);
            REQUIRE(intersect);
            REQUIRE(*intersect == Approx(10.0f));
        }

        SECTION("hits at the corner of the triangle")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            Ray3f r(
                Vector3f(-1.0f, -1.0f, 10.0f),
                Vector3f(0.0f, 0.0f, -1.0f));
            auto intersect = tri.intersect(r);
            REQUIRE(intersect);
            REQUIRE(*intersect == Approx(10.0f));
        }

        SECTION("misses just below the corner of the triangle")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            Ray3f r(
                Vector3f(-1.0f, -1.000001f, 10.0f),
                Vector3f(0.0f, 0.0f, -1.0f));
            auto intersect = tri.intersect(r);
            REQUIRE(!intersect);
        }

        SECTION("returns a miss when the ray misses")
        {
            SECTION("case 1")
            {
                Triangle3f tri(
                    Vector3f(-1.0f, -1.0f, 0.0f),
                    Vector3f(1.0f, -1.0f, 0.0f),
                    Vector3f(0.0f, 1.0f, 0.0f));
                Ray3f r(
                    Vector3f(2.0f, 2.0f, 10.0f),
                    Vector3f(0.0f, 0.0f, -1.0f));
                REQUIRE(!tri.intersect(r));
            }

            SECTION("case 2")
            {
                Triangle3f tri(
                    Vector3f(1.0f, 1.0f, 0.0f),
                    Vector3f(3.0f, 1.0f, 0.0f),
                    Vector3f(0.0f, 3.0f, 0.0f));
                Ray3f r(
                    Vector3f(0.0f, 0.0f, 10.0f),
                    Vector3f(0.0f, 0.0f, -1.0f));
                REQUIRE(!tri.intersect(r));
            }

            SECTION("when ray is parallel above the triangle")
            {
                Triangle3f tri(
                    Vector3f(1.0f, 1.0f, 0.0f),
                    Vector3f(3.0f, 1.0f, 0.0f),
                    Vector3f(0.0f, 3.0f, 0.0f));
                Ray3f r(
                    Vector3f(0.0f, 0.0f, 10.0f),
                    Vector3f(0.0f, 1.0f, 0.0f));
                REQUIRE(!tri.intersect(r));
            }

            SECTION("when ray is parallel below the triangle")
            {
                Triangle3f tri(
                    Vector3f(1.0f, 1.0f, 0.0f),
                    Vector3f(3.0f, 1.0f, 0.0f),
                    Vector3f(0.0f, 3.0f, 0.0f));
                Ray3f r(
                    Vector3f(0.0f, 0.0f, -10.0f),
                    Vector3f(0.0f, 1.0f, 0.0f));
                REQUIRE(!tri.intersect(r));
            }
        }
    }

    TEST_CASE("Triangle3f::intersectLine")
    {
        SECTION("returns the point when the line hits")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = tri.intersectLine(Vector3f(0.0f, 0.0f, 10.0f), Vector3f(0.0f, 0.0f, -10.0f));
            REQUIRE(intersect);
            REQUIRE(intersect->x == Approx(0.0f));
            REQUIRE(intersect->y == Approx(0.0f));
            REQUIRE(intersect->z == Approx(0.0f));
        }

        SECTION("works for a line in the other direction")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = tri.intersectLine(Vector3f(0.0f, 0.0f, -10.0f), Vector3f(0.0f, 0.0f, 10.0f));
            REQUIRE(intersect);
            REQUIRE(intersect->x == Approx(0.0f));
            REQUIRE(intersect->y == Approx(0.0f));
            REQUIRE(intersect->z == Approx(0.0f));
        }

        SECTION("hits at the corner of the triangle")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = tri.intersectLine(Vector3f(-1.0f, -1.0f, 10.0f), Vector3f(-1.0f, -1.0f, -10.0f));
            REQUIRE(intersect);
            REQUIRE(intersect->x == Approx(-1.0f));
            REQUIRE(intersect->y == Approx(-1.0f));
            REQUIRE(intersect->z == Approx(0.0f));
        }

        SECTION("misses just below the corner of the triangle")
        {
            Triangle3f tri(
                Vector3f(-1.0f, -1.0f, 0.0f),
                Vector3f(1.0f, -1.0f, 0.0f),
                Vector3f(0.0f, 1.0f, 0.0f));
            auto intersect = tri.intersectLine(Vector3f(-1.0f, -1.000001f, 10.0f), Vector3f(-1.0f, -1.000001f, -10.0f));
            REQUIRE(!intersect);
        }
    }

    TEST_CASE("Triangle3f::toPlane")
    {
        SECTION("returns the plane the triangle lies on")
        {
            Triangle3f tri(
                Vector3f(-1, -1, 0),
                Vector3f(1, -1, 0),
                Vector3f(0, 1, 0));
            Plane3f p = tri.toPlane();

            REQUIRE(p.point.x == Approx(-1.0f));
            REQUIRE(p.point.y == Approx(-1.0f));
            REQUIRE(p.point.z == Approx(0.0f));

            REQUIRE(p.normal.x == Approx(0.0f));
            REQUIRE(p.normal.y == Approx(0.0f));
            REQUIRE(p.normal.z == Approx(4.0f));
        }
    }
}
