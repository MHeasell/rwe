#include <catch2/catch.hpp>
#include <rapidcheck.h>
#include <rapidcheck/catch.h>
#include <rwe/geometry/Circle2f.h>
#include <rwe/math/Vector2f.h>
#include <rwe/util.h>

namespace rwe
{
    TEST_CASE("Circle2f::contains")
    {
        SECTION("returns true when point is inside the circle")
        {
            Circle2f c(10.0f, Vector2f(1.0f, 2.0f));

            // x
            REQUIRE(c.contains(Vector2f(11.0f, 2.0f)));
            REQUIRE(!c.contains(Vector2f(11.25f, 2.0f)));
            REQUIRE(c.contains(Vector2f(-9.0f, 2.0f)));
            REQUIRE(!c.contains(Vector2f(-9.25f, 2.0f)));

            // y
            REQUIRE(c.contains(Vector2f(1.0f, 12.0f)));
            REQUIRE(!c.contains(Vector2f(1.0f, 12.25f)));
            REQUIRE(c.contains(Vector2f(1.0f, -8.0f)));
            REQUIRE(!c.contains(Vector2f(1.0f, -8.25f)));

            // diagonal
            REQUIRE(c.contains(Vector2f(8.0f, 9.0f)));
            REQUIRE(!c.contains(Vector2f(8.5f, 9.5f)));
        }

        rc::prop("works when point is within radius distance on axes", []() {
            float x = static_cast<float>(*rc::gen::inRange(-10000, 10000));
            float y = static_cast<float>(*rc::gen::inRange(-10000, 10000));
            int radius = *rc::gen::inRange(0, 10000);
            float distance = static_cast<float>(*rc::gen::inRange(-radius, radius + 1));
            Circle2f c(static_cast<float>(radius), Vector2f(x, y));
            Vector2f px(x + distance, y);
            Vector2f py(x, y + distance);
            RC_ASSERT(c.contains(px));
            RC_ASSERT(c.contains(py));
        });
        rc::prop("works when point is within sqrt(radius) distance on axes", []() {
            auto x = static_cast<float>(*rc::gen::inRange(-10000, 10000));
            auto y = static_cast<float>(*rc::gen::inRange(-10000, 10000));
            auto radius = *rc::gen::inRange(0, 10000);
            auto dx = static_cast<float>(*rc::gen::inRange(-radius, radius + 1));
            auto dy = static_cast<float>(*rc::gen::inRange(-radius, radius + 1));

            const auto scale = std::sin(Pif / 4.f);
            Circle2f c(static_cast<float>(radius), Vector2f(x, y));
            Vector2f p(x + (dx * scale * 0.9f), y + (dy * scale * 0.9f));
            RC_ASSERT(c.contains(p));
        });
    }
}
