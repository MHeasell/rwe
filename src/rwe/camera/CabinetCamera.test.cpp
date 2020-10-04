#include <catch2/catch.hpp>
#include <rwe/camera/CabinetCamera.h>

namespace rwe
{
    TEST_CASE("CabinetCamera")
    {
        SECTION("matices and inverses cancel out")
        {
            CabinetCamera cam(640.0f, 480.0f);
            auto m = cam.getViewProjectionMatrix() * cam.getInverseViewProjectionMatrix();

            REQUIRE(m.data[0] == Approx(1.0f));
            REQUIRE(m.data[1] == Approx(0.0f));
            REQUIRE(m.data[2] == Approx(0.0f));
            REQUIRE(m.data[3] == Approx(0.0f));

            REQUIRE(m.data[4] == Approx(0.0f));
            REQUIRE(m.data[5] == Approx(1.0f));
            REQUIRE(m.data[6] == Approx(0.0f));
            REQUIRE(m.data[7] == Approx(0.0f));

            REQUIRE(m.data[8] == Approx(0.0f));
            REQUIRE(m.data[9] == Approx(0.0f));
            REQUIRE(m.data[10] == Approx(1.0f));
            REQUIRE(m.data[11] == Approx(0.0f));

            REQUIRE(m.data[12] == Approx(0.0f));
            REQUIRE(m.data[13] == Approx(0.0f));
            REQUIRE(m.data[14] == Approx(0.0f));
            REQUIRE(m.data[15] == Approx(1.0f));
        }

        SECTION("matices and inverses cancel out when the camera is moved")
        {
            CabinetCamera cam(640.0f, 480.0f);
            cam.translate(Vector3f(5.0f, 6.0f, 7.0f));

            auto m = cam.getViewProjectionMatrix() * cam.getInverseViewProjectionMatrix();

            REQUIRE(m.data[0] == Approx(1.0f));
            REQUIRE(m.data[1] == Approx(0.0f));
            REQUIRE(m.data[2] == Approx(0.0f));
            REQUIRE(m.data[3] == Approx(0.0f));

            REQUIRE(m.data[4] == Approx(0.0f));
            REQUIRE(m.data[5] == Approx(1.0f));
            REQUIRE(m.data[6] == Approx(0.0f));
            REQUIRE(m.data[7] == Approx(0.0f));

            REQUIRE(m.data[8] == Approx(0.0f));
            REQUIRE(m.data[9] == Approx(0.0f));
            REQUIRE(m.data[10] == Approx(1.0f));
            REQUIRE(m.data[11] == Approx(0.0f));

            REQUIRE(m.data[12] == Approx(0.0f));
            REQUIRE(m.data[13] == Approx(0.0f));
            REQUIRE(m.data[14] == Approx(0.0f));
            REQUIRE(m.data[15] == Approx(1.0f));
        }
    }
}
