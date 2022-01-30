#include <catch2/catch.hpp>
#include <rwe/io/featuretdf/io.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/optional_io.h>

namespace rwe
{
    std::ostream& operator<<(std::ostream& os, const TdfBlock& b)
    {
        os << "<block with " << b.properties.size() << " properties and " << b.blocks.size() << "sub-blocks>";
        return os;
    }

    TEST_CASE("parseFeatureDefinition")
    {
        SECTION("basic test")
        {
            std::string input = R"TDF(
[Tree1]
    {
    world=greenworld;
    description=Tree;
    category=trees;
    animating=0;
    footprintx=4;
    footprintz=5;
    height=40;
    filename=trees;
    seqname=leaf1;
    seqnameshad=leafshad;
    seqnamereclamate=tree1reclamate;
    energy = 250;

    seqnameburn=leafyburn01;
    seqnameburnshad=;
    featureburnt=Tree1Dead;
    burnmin=5;
    burnmax=15;
    sparktime=5;
    spreadchance=90;
    burnweapon=TreeBurn;

    seqnamedie=treeboom;
    featuredead=smudge01;
    featurereclamate=smudge01;

    animtrans=0;
    shadtrans=1;
    flamable=1;
    reclaimable=1;
    reproduce=0;
    reproducearea=6;
    blocking=1;
    hitdensity=10;
    }
)TDF";

            auto tdfRoot = parseTdfFromString(input);
            REQUIRE(tdfRoot.blocks.size() == 1);
            REQUIRE(tdfRoot.properties.size() == 0);

            auto block = tdfRoot.findBlock("Tree1");
            REQUIRE(block);
            auto f = parseFeatureDefinition(*block);

            REQUIRE(f.world == "greenworld");
            REQUIRE(f.description == "Tree");
            REQUIRE(f.category == "trees");
            REQUIRE(f.footprintX == 4);
            REQUIRE(f.footprintZ == 5);
            REQUIRE(f.height == 40);
            REQUIRE(f.fileName == "trees");
            REQUIRE(f.seqName == "leaf1");
            REQUIRE(f.seqNameShad == "leafshad");
            REQUIRE(f.seqNameReclamate == "tree1reclamate");
            REQUIRE(f.energy == 250);

            REQUIRE(f.seqNameBurn == "leafyburn01");
            REQUIRE(f.seqNameBurnShad == "");
            REQUIRE(f.featureBurnt == "Tree1Dead");
            REQUIRE(f.burnMin == 5);
            REQUIRE(f.burnMax == 15);
            REQUIRE(f.sparkTime == 5);
            REQUIRE(f.spreadChance == 90);
            REQUIRE(f.burnWeapon == "TreeBurn");

            REQUIRE(f.seqNameDie == "treeboom");
            REQUIRE(f.featureDead == "smudge01");
            REQUIRE(f.featureReclamate == "smudge01");

            REQUIRE(f.animTrans == false);
            REQUIRE(f.shadTrans == true);
            REQUIRE(f.flamable == true);
            REQUIRE(f.reclaimable == true);
            REQUIRE(f.reproduce == false);
            REQUIRE(f.reproduceArea == 6);

            REQUIRE(f.blocking == 1);
            REQUIRE(f.hitDensity == 10);

            // fields not provided
            REQUIRE(f.animating == false);
            REQUIRE(f.object == "");
            REQUIRE(f.autoreclaimable == true);
            REQUIRE(f.metal == 0);
            REQUIRE(f.geothermal == false);
            REQUIRE(f.noDisplayInfo == false);
            REQUIRE(f.permanent == false);
            REQUIRE(f.blocking == true);
            REQUIRE(f.indestructible == false);
            REQUIRE(f.damage == 1);
        }
    }
}
