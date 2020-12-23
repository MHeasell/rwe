#include <catch2/catch.hpp>
#include <rwe/GameScene_util.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, SimScalar r)
    {
        os << r.value;
        return os;
    }

    TEST_CASE("getPieceTransform")
    {
        SECTION("works")
        {
            std::vector<UnitPieceDefinition> pieceDefs{UnitPieceDefinition{"foo", SimVector(0_ss, 0_ss, 0_ss)}};
            std::vector<UnitMesh> pieces{UnitMesh{"foo"}};
            REQUIRE(getPieceTransform("foo", pieceDefs, pieces) == Matrix4x<SimScalar>::identity());
        }

        SECTION("works when piece has non-zero origin")
        {
            std::vector<UnitPieceDefinition> pieceDefs{UnitPieceDefinition{"foo", SimVector(1_ss, 2_ss, 3_ss)}};
            std::vector<UnitMesh> pieces{UnitMesh{"foo"}};
            auto expected = Matrix4x<SimScalar>::translation(SimVector(1_ss, 2_ss, 3_ss));
            REQUIRE(getPieceTransform("foo", pieceDefs, pieces) == expected);
        }

        SECTION("works when piece has non-zero origin and is translated")
        {
            std::vector<UnitPieceDefinition> pieceDefs{UnitPieceDefinition{"foo", SimVector(1_ss, 2_ss, 3_ss)}};
            std::vector<UnitMesh> pieces{UnitMesh{"foo"}};
            pieces[0].offset = SimVector(10_ss, 20_ss, 30_ss);
            auto expected = Matrix4x<SimScalar>::translation(SimVector(11_ss, 22_ss, 33_ss));
            REQUIRE(getPieceTransform("foo", pieceDefs, pieces) == expected);
        }

        SECTION("works with child pieces translated")
        {
            std::vector<UnitPieceDefinition> pieceDefs{
                UnitPieceDefinition{"foo", SimVector(1_ss, 2_ss, 3_ss)},
                UnitPieceDefinition{"bar", SimVector(2_ss, 3_ss, 4_ss), "foo"},
            };
            std::vector<UnitMesh> pieces{UnitMesh{"foo"}, UnitMesh{"bar"}};
            pieces[0].offset = SimVector(10_ss, 20_ss, 30_ss);
            pieces[1].offset = SimVector(100_ss, 200_ss, 300_ss);

            auto fooExpected = Matrix4x<SimScalar>::translation(SimVector(11_ss, 22_ss, 33_ss));
            REQUIRE(getPieceTransform("foo", pieceDefs, pieces) == fooExpected);

            auto barExpected = Matrix4x<SimScalar>::translation(SimVector(113_ss, 225_ss, 337_ss));
            REQUIRE(getPieceTransform("bar", pieceDefs, pieces) == barExpected);
        }

        SECTION("works with child piece rotated")
        {
            std::vector<UnitPieceDefinition> pieceDefs{
                UnitPieceDefinition{"foo", SimVector(1_ss, 2_ss, 3_ss)},
                UnitPieceDefinition{"bar", SimVector(10_ss, 20_ss, 30_ss), "foo"},
            };
            std::vector<UnitMesh> pieces{UnitMesh{"foo"}, UnitMesh{"bar"}};
            pieces[0].rotationY = QuarterTurn;

            auto fooExpectedPosition = SimVector(1_ss, 2_ss, 3_ss);
            REQUIRE(getPieceTransform("foo", pieceDefs, pieces) * SimVector(0_ss, 0_ss, 0_ss) == fooExpectedPosition);

            // sim scalars are still backed by floats atm, so rotation by 90deg
            // still introduces a bit of fp error due to conversion to radians
            auto barExpectedPosition = SimVector(31_ss, 22_ss, -7_ss);
            auto barActualPosition = getPieceTransform("bar", pieceDefs, pieces) * SimVector(0_ss, 0_ss, 0_ss);
            REQUIRE(barActualPosition.x.value == Approx(barExpectedPosition.x.value));
            REQUIRE(barActualPosition.y.value == Approx(barExpectedPosition.y.value));
            REQUIRE(barActualPosition.z.value == Approx(barExpectedPosition.z.value));
        }
    }
}
