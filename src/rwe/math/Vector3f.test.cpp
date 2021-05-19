#include <catch2/catch.hpp>
#include <rwe/math/Vector3f.h>
#include <rwe/optional_io.h>
#include <rwe/util.h>
#include <sstream>

namespace rwe
{
    TEST_CASE("Vector3f::cross")
    {
        SECTION("returns the cross product of two simple vectors")
        {
            Vector3f a(1.0f, 0.0f, 0.0f);
            Vector3f b(0.0f, 1.0f, 0.0f);
            Vector3f c = a.cross(b);

            REQUIRE(c.x == 0.0f);
            REQUIRE(c.y == 0.0f);
            REQUIRE(c.z == 1.0f);
        }
    }

    TEST_CASE("Vector3f::dot")
    {
        SECTION("returns 0 for perpendicular vectors")
        {
            Vector3f a(1.0f, 0.0f, 0.0f);
            Vector3f b(0.0f, 1.0f, 0.0f);
            REQUIRE(a.dot(b) == 0.0f);
        }

        SECTION("computes dot product")
        {
            Vector3f a(1.0f, 2.0f, 3.0f);
            Vector3f b(4.0f, 5.0f, 6.0f);
            REQUIRE(a.dot(b) == 32.0f);
        }
    }
    TEST_CASE("Vector3f::operator<<")
    {
        SECTION("prints the vector to the stream")
        {
            Vector3f v(0.5f, 1.0f, 1.5f);
            std::ostringstream os;
            os << v;
            REQUIRE(os.str() == "(0.5, 1, 1.5)");
        }
    }

    TEST_CASE("Vector3f::normalized")
    {
        SECTION("returns a new normalized vector")
        {
            Vector3f v(3.0, 4.0, 0.0);
            Vector3f n = v.normalized();
            REQUIRE(n.x == Approx(0.6f));
            REQUIRE(n.y == Approx(0.8f));
            REQUIRE(n.z == Approx(0.0f));
        }
    }

    TEST_CASE("Vector3f::normalizedOr")
    {
        SECTION("when length is non-zero, returns a new normalized vector")
        {
            Vector3f v(3.0, 4.0, 0.0);
            Vector3f n = v.normalizedOr(Vector3f(1.0f, 0.0f, 0.0f));
            REQUIRE(n.x == Approx(0.6f));
            REQUIRE(n.y == Approx(0.8f));
            REQUIRE(n.z == Approx(0.0f));
        }
        SECTION("when length is zero, returns the default value")
        {
            Vector3f v(0.0, 0.0, 0.0);
            Vector3f n = v.normalizedOr(Vector3f(1.0f, 0.0f, 0.0f));
            REQUIRE(n.x == 1.0f);
            REQUIRE(n.y == 0.0f);
            REQUIRE(n.z == 0.0f);
        }
    }

    TEST_CASE("closestTo")
    {
        SECTION("returns a when b is not defined")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            std::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            std::optional<Vector3f> b;

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 1.0f);
            REQUIRE(result->y == 2.0f);
            REQUIRE(result->z == 3.0f);
        }

        SECTION("returns b when a is not defined")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            std::optional<Vector3f> a;
            std::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 2.0f);
            REQUIRE(result->y == 3.0f);
            REQUIRE(result->z == 4.0f);
        }

        SECTION("when both are defined, returns the closer")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            std::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            std::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 1.0f);
            REQUIRE(result->y == 2.0f);
            REQUIRE(result->z == 3.0f);
        }

        SECTION("when both are defined, returns the closer (test 2)")
        {
            Vector3f origin(3.0f, 3.0f, 3.0f);
            std::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            std::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 2.0f);
            REQUIRE(result->y == 3.0f);
            REQUIRE(result->z == 4.0f);
        }
    }

    TEST_CASE("operator-")
    {
        SECTION("negates the vector")
        {
            Vector3f a(1.0f, 2.0f, 3.0f);
            Vector3f b = -a;
            REQUIRE(b == Vector3f(-1.0f, -2.0f, -3.0f));
        }
    }

    TEST_CASE("Vector3f::angleTo")
    {
        SECTION("works for parallel vectors")
        {
            SECTION("X")
            {
                Vector3f a(1.0f, 0.0f, 0.0f);
                Vector3f b(1.0f, 0.0f, 0.0f);
                Vector3f n(0.0f, 0.0f, 1.0f);
                REQUIRE(angleTo(a, b, n) == Approx(0.0f));
            }
            SECTION("Y")
            {
                Vector3f a(0.0f, 1.0f, 0.0f);
                Vector3f b(0.0f, 1.0f, 0.0f);
                Vector3f n(1.0f, 0.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(0.0f));
            }
            SECTION("Z")
            {
                Vector3f a(0.0f, 0.0f, 1.0f);
                Vector3f b(0.0f, 0.0f, 1.0f);
                Vector3f n(0.0f, 1.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(0.0f));
            }
        }
        SECTION("works for perpendicular vectors")
        {
            SECTION("X -> Y")
            {
                Vector3f a(1.0f, 0.0f, 0.0f);
                Vector3f b(0.0f, 1.0f, 0.0f);
                Vector3f n(0.0f, 0.0f, 1.0f);
                REQUIRE(angleTo(a, b, n) == Approx(Pif / 2.0f));
            }
            SECTION("Z -> X")
            {
                Vector3f a(0.0f, 0.0f, 1.0f);
                Vector3f b(1.0f, 0.0f, 0.0f);
                Vector3f n(0.0f, 1.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(Pif / 2.0f));
            }
            SECTION("Y -> Z")
            {
                Vector3f a(0.0f, 1.0f, 0.0f);
                Vector3f b(0.0f, 0.0f, 1.0f);
                Vector3f n(1.0f, 0.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(Pif / 2.0f));
            }
        }
        SECTION("works for perpendicular vectors with negative angle")
        {
            SECTION("Y -> X")
            {
                Vector3f a(0.0f, 1.0f, 0.0f);
                Vector3f b(1.0f, 0.0f, 0.0f);
                Vector3f n(0.0f, 0.0f, 1.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif / 2.0f));
            }
            SECTION("X -> Z")
            {
                Vector3f a(1.0f, 0.0f, 0.0f);
                Vector3f b(0.0f, 0.0f, 1.0f);
                Vector3f n(0.0f, 1.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif / 2.0f));
            }
            SECTION("Z -> Y")
            {
                Vector3f a(0.0f, 0.0f, 1.0f);
                Vector3f b(0.0f, 1.0f, 0.0f);
                Vector3f n(1.0f, 0.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif / 2.0f));
            }
        }
        SECTION("works for opposite vectors")
        {
            SECTION("X")
            {
                Vector3f a(1.0f, 0.0f, 0.0f);
                Vector3f b(-1.0f, 0.0f, 0.0f);
                Vector3f n(0.0f, 0.0f, 1.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif));
            }

            SECTION("Y")
            {
                Vector3f a(0.0f, 1.0f, 0.0f);
                Vector3f b(0.0f, -1.0f, 0.0f);
                Vector3f n(1.0f, 0.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif));
            }

            SECTION("Z")
            {
                Vector3f a(0.0f, 0.0f, 1.0f);
                Vector3f b(0.0f, 0.0f, -1.0f);
                Vector3f n(0.0f, 1.0f, 0.0f);
                REQUIRE(angleTo(a, b, n) == Approx(-Pif));
            }
        }
        SECTION("works for vectors not of the same length")
        {
            Vector3f a(0.0f, 1.0f, 0.0f);
            Vector3f b(-1.0f, 1.0f, 0.0f);
            Vector3f n(-1.0f, 1.0f, 1.0f);
            REQUIRE(angleTo(a, b, n) == Approx(Pif / 4.0f));
        }
        SECTION("it doesnt emit nan")
        {
            // observed in playtesting,
            // this can cause NaN to be emitted by acos
            // due to floating point error in the input
            Vector3f a(171.99025f, -0.00849914551f, -38.2367249f);
            Vector3f b(171.99025f, 0.f, -38.2367249f);
            Vector3f n(38.2367249f, -0.f, 171.99025f);
            REQUIRE(angleTo(a, b, n) == Approx(0.0f));
        }
    }

    TEST_CASE("determinant")
    {
        Vector3f a(6.0f, 4.0f, 2.0f);
        Vector3f b(1.0f, -2.0f, 8.0f);
        Vector3f c(1.0f, 5.0f, 7.0f);
        REQUIRE(determinant(a, b, c) == -306.0f);
    }

    TEST_CASE("xy")
    {
        SECTION("returns the X and Y components")
        {
            Vector3f v(1.0f, 2.0f, 3.0f);
            REQUIRE(v.xy() == Vector2f(1.0f, 2.0f));
        }
    }

    TEST_CASE("xz")
    {
        SECTION("returns the X and Z components")
        {
            Vector3f v(1.0f, 2.0f, 3.0f);
            REQUIRE(v.xz() == Vector2f(1.0f, 3.0f));
        }
    }
}
