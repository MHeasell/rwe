#include <catch2/catch.hpp>
#include <rwe/math/Matrix4f.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    TEST_CASE("Matrix4f operator*")
    {
        SECTION("multiplies matrices")
        {
            // 2x scale matrix
            Matrix4f a = Matrix4f::scale(2.0f);

            // translation matrix
            Matrix4f b = Matrix4f::translation(Vector3f(4.0f, 5.0f, 6.0f));

            // Result should be translate then scale.
            // That is, the translation would be applied first,
            // then the result scaled.
            // This effectively scales the translation distance.
            Matrix4f c = a * b;

            REQUIRE(c.data[0] == 2.0f);
            REQUIRE(c.data[1] == 0.0f);
            REQUIRE(c.data[2] == 0.0f);
            REQUIRE(c.data[3] == 0.0f);

            REQUIRE(c.data[4] == 0.0f);
            REQUIRE(c.data[5] == 2.0f);
            REQUIRE(c.data[6] == 0.0f);
            REQUIRE(c.data[7] == 0.0f);

            REQUIRE(c.data[8] == 0.0f);
            REQUIRE(c.data[9] == 0.0f);
            REQUIRE(c.data[10] == 2.0f);
            REQUIRE(c.data[11] == 0.0f);

            REQUIRE(c.data[12] == 8.0f);
            REQUIRE(c.data[13] == 10.0f);
            REQUIRE(c.data[14] == 12.0f);
            REQUIRE(c.data[15] == 1.0f);
        }
    }

    TEST_CASE("Matrix4f operator==")
    {
        SECTION("tests equality")
        {
            auto a = Matrix4f::identity();
            auto b = Matrix4f::identity();

            REQUIRE(a == b);

            b.data[15] = 2.0f;

            REQUIRE(a != b);
        }
    }

    TEST_CASE("Matrix4f::orthographicProjection")
    {
        SECTION("combines with inverse to produce the identity")
        {
            Matrix4f a = Matrix4f::orthographicProjection(-40.0f, 60.0f, -30.0f, 10.0f, 2.0f, 200.0f);
            Matrix4f b = Matrix4f::inverseOrthographicProjection(-40.0f, 60.0f, -30.0f, 10.0f, 2.0f, 200.0f);
            Matrix4f c = b * a;

            REQUIRE(c.data[0] == Approx(1.0f));
            REQUIRE(c.data[1] == Approx(0.0f));
            REQUIRE(c.data[2] == Approx(0.0f));
            REQUIRE(c.data[3] == Approx(0.0f));

            REQUIRE(c.data[4] == Approx(0.0f));
            REQUIRE(c.data[5] == Approx(1.0f));
            REQUIRE(c.data[6] == Approx(0.0f));
            REQUIRE(c.data[7] == Approx(0.0f));

            REQUIRE(c.data[8] == Approx(0.0f));
            REQUIRE(c.data[9] == Approx(0.0f));
            REQUIRE(c.data[10] == Approx(1.0f));
            REQUIRE(c.data[11] == Approx(0.0f));

            REQUIRE(c.data[12] == Approx(0.0f));
            REQUIRE(c.data[13] == Approx(0.0f));
            REQUIRE(c.data[14] == Approx(0.0f));
            REQUIRE(c.data[15] == Approx(1.0f));
        }
    }

    TEST_CASE("Matrix4f::cabinetProjection")
    {
        SECTION("Produces the identity when combined with opposite")
        {
            Matrix4f a = Matrix4f::cabinetProjection(0.0f, 0.5f);
            Matrix4f b = Matrix4f::cabinetProjection(0.0f, -0.5f);
            Matrix4f c = b * a;

            REQUIRE(c.data[0] == Approx(1.0f));
            REQUIRE(c.data[1] == Approx(0.0f));
            REQUIRE(c.data[2] == Approx(0.0f));
            REQUIRE(c.data[3] == Approx(0.0f));

            REQUIRE(c.data[4] == Approx(0.0f));
            REQUIRE(c.data[5] == Approx(1.0f));
            REQUIRE(c.data[6] == Approx(0.0f));
            REQUIRE(c.data[7] == Approx(0.0f));

            REQUIRE(c.data[8] == Approx(0.0f));
            REQUIRE(c.data[9] == Approx(0.0f));
            REQUIRE(c.data[10] == Approx(1.0f));
            REQUIRE(c.data[11] == Approx(0.0f));

            REQUIRE(c.data[12] == Approx(0.0f));
            REQUIRE(c.data[13] == Approx(0.0f));
            REQUIRE(c.data[14] == Approx(0.0f));
            REQUIRE(c.data[15] == Approx(1.0f));
        }
    }

    TEST_CASE("Matrix4f::translation")
    {
        SECTION("Produces a translation matrix")
        {
            Matrix4f m = Matrix4f::translation(Vector3f(3.0f, 4.0f, 5.0f));
            Vector3f v(11.0f, 15.0f, 19.0f);
            auto u = m * v;
            REQUIRE(u.x == Approx(14.0f));
            REQUIRE(u.y == Approx(19.0f));
            REQUIRE(u.z == Approx(24.0f));
        }
    }

    TEST_CASE("Matrix4f::transposed")
    {
        SECTION("transposes a matrix")
        {
            Matrix4f a;

            a.data[0] = 0;
            a.data[1] = 1;
            a.data[2] = 2;
            a.data[3] = 3;

            a.data[4] = 4;
            a.data[5] = 5;
            a.data[6] = 6;
            a.data[7] = 7;

            a.data[8] = 8;
            a.data[9] = 9;
            a.data[10] = 10;
            a.data[11] = 11;

            a.data[12] = 12;
            a.data[13] = 13;
            a.data[14] = 14;
            a.data[15] = 15;

            Matrix4f b = a.transposed();

            REQUIRE(b.data[0] == 0);
            REQUIRE(b.data[1] == 4);
            REQUIRE(b.data[2] == 8);
            REQUIRE(b.data[3] == 12);

            REQUIRE(b.data[4] == 1);
            REQUIRE(b.data[5] == 5);
            REQUIRE(b.data[6] == 9);
            REQUIRE(b.data[7] == 13);

            REQUIRE(b.data[8] == 2);
            REQUIRE(b.data[9] == 6);
            REQUIRE(b.data[10] == 10);
            REQUIRE(b.data[11] == 14);

            REQUIRE(b.data[12] == 3);
            REQUIRE(b.data[13] == 7);
            REQUIRE(b.data[14] == 11);
            REQUIRE(b.data[15] == 15);
        }
    }
}
