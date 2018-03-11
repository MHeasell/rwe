#include <rwe/optional_io.h>
#include <catch.hpp>
#include <rwe/geometry/Ray3f.h>
#include <rwe/geometry/CollisionMesh.h>

namespace rwe
{
    TEST_CASE("CollisionMesh")
    {
        SECTION("fromQuad")
        {
            SECTION("parses from a quad")
            {
                auto cm = CollisionMesh::fromQuad(
                    Vector3f(-1.0f, 0.0f, -1.0f),
                    Vector3f( 1.0f, 0.0f, -1.0f),
                    Vector3f( 1.0f, 0.0f,  1.0f),
                    Vector3f(-1.0f, 0.0f,  1.0f));

                Ray3f ray(
                    Vector3f(0.0f, 10.0f, 0.0f),
                    Vector3f(0.0f, -1.0f, 0.0f));

                REQUIRE(cm.intersect(ray));
            }
        }
    }
}
