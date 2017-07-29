#include <catch.hpp>
#include <rwe/math/Vector3f.h>

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
        SECTION("prints the rwe_math to the stream")
        {
            Vector3f v(0.5f, 1.0f, 1.5f);
            std::ostringstream os;
            os << v;
            REQUIRE(os.str() == "(0.5, 1, 1.5)");
        }
    }

    TEST_CASE("Vector3f::normalized")
    {
        SECTION("returns a new normalized rwe_math")
        {
            Vector3f v(3.0, 4.0, 0.0);
            Vector3f n = v.normalized();
            REQUIRE(n.x == Approx(0.6f));
            REQUIRE(n.y == Approx(0.8f));
            REQUIRE(n.z == Approx(0.0f));
        }
    }

    TEST_CASE("closestTo")
    {
        SECTION("returns a when b is not defined")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            boost::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            boost::optional<Vector3f> b;

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 1.0f);
            REQUIRE(result->y == 2.0f);
            REQUIRE(result->z == 3.0f);
        }

        SECTION("returns b when a is not defined")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            boost::optional<Vector3f> a;
            boost::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 2.0f);
            REQUIRE(result->y == 3.0f);
            REQUIRE(result->z == 4.0f);
        }

        SECTION("when both are defined, returns the closer")
        {
            Vector3f origin(0.0f, 0.0f, 0.0f);
            boost::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            boost::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 1.0f);
            REQUIRE(result->y == 2.0f);
            REQUIRE(result->z == 3.0f);
        }

        SECTION("when both are defined, returns the closer (test 2)")
        {
            Vector3f origin(3.0f, 3.0f, 3.0f);
            boost::optional<Vector3f> a = Vector3f(1.0f, 2.0f, 3.0f);
            boost::optional<Vector3f> b = Vector3f(2.0f, 3.0f, 4.0f);

            auto result = closestTo(origin, a, b);
            REQUIRE(result);
            REQUIRE(result->x == 2.0f);
            REQUIRE(result->y == 3.0f);
            REQUIRE(result->z == 4.0f);
        }
    }
}
