#include <catch.hpp>
#include <rwe/SideData.h>
#include <rwe/tdf.h>

namespace rwe
{
    TEST_CASE("parseSideData")
    {
        SECTION("parses a sidedata block")
        {
            std::string input = R"TDF(
[SIDE0]
	{
	name=CoolFaction;
	nameprefix=COO;
	commander=ARMCOM;
	intgaf=ARMINT;

	font=console;
	fontgui=armbutt;
	energycolor=208;
	metalcolor=224;

	[LOGO]
		{ x1=132; y1=5; x2=152; y2=25; }

	[ENERGYBAR]
		{ x1=471; y1=11; x2=592; y2=13; }

	[ENERGYNUM]
		{ x1=529; y1=18; x2=529; y2=18; }

	[ENERGYMAX]
		{ x1=595; y1=1; x2=595; y2=1; }

	[ENERGY0]
		{ x1=468; y1=1; x2=468; y2=1; }

	[METALBAR]
		{ x1=218; y1=11; x2=339; y2=13; }

	[METALNUM]
		{ x1=278; y1=18; x2=278; y2=18; }

	[METALMAX]
		{ x1=341; y1=1; x2=341; y2=1; }

	[METAL0]
		{ x1=215; y1=1; x2=215; y2=1; }

	[TOTALUNITS]
		{ x1=560; y1=7; x2=583; y2=15; }

	[TOTALTIME]
		{ x1=605; y1=7; x2=628; y2=15; }

	[ENERGYPRODUCED]
		{ x1=609; y1=5; x2=609; y2=5; }

	[ENERGYCONSUMED]
		{ x1=609; y1=17; x2=609; y2=17; }

	[METALPRODUCED]
		{ x1=358; y1=5; x2=358; y2=5; }

	[METALCONSUMED]
		{ x1=358; y1=17; x2=358; y2=17; }

	// All of these regions describe areas on the screen footer

	[LOGO2]
		{ x1=132; y1=455; x2=152; y2=475; }

	[UNITNAME]
		{ x1=245; y1=452; x2=245; y2=460; }

	[DAMAGEBAR]
		{ x1=200; y1=463; x2=290; y2=465; }

	[UNITMETALMAKE]
		{ x1=350; y1=458; x2=170; y2=478; }

	[UNITMETALUSE]
		{ x1=350; y1=468; x2=170; y2=478; }

	[UNITENERGYMAKE]
		{ x1=400; y1=458; x2=170; y2=468; }

	[UNITENERGYUSE]
		{ x1=400; y1=468; x2=170; y2=468; }

	[MISSIONTEXT]
		{ x1=385; y1=449; x2=400; y2=450; }

	[UNITNAME2]
		{ x1=555; y1=452; x2=555; y2=460; }

	[DAMAGEBAR2]
		{ x1=510; y1=463; x2=600; y2=465; }

	[NAME]
		{ x1=132; y1=452; x2=142; y2=460; }

	[DESCRIPTION]
		{ x1=132; y1=465; x2=142; y2=472; }

	[RELOAD1]
		{ x1=132; y1=450; x2=148; y2=458; }

	[RELOAD2]
		{ x1=132; y1=460; x2=148; y2=468; }

	[RELOAD3]
		{ x1=132; y1=470; x2=148; y2=478; }
	}
)TDF";

            auto tdf = parseTdfFromString(input);
            auto block = tdf.findBlock("SIDE0");
            if (!block)
            {
                FAIL();
            }

            auto s = parseSideData(*block);


            REQUIRE(s.name == "CoolFaction");
            REQUIRE(s.namePrefix == "COO");
            REQUIRE(s.commander == "ARMCOM");
            REQUIRE(s.intGaf == "ARMINT");
            REQUIRE(s.font == "console");
            REQUIRE(s.fontGui == "armbutt");
            REQUIRE(s.energyColor == 208);
            REQUIRE(s.metalColor == 224);

            REQUIRE(s.logo == SideDataRect(132, 5, 152, 25));
            REQUIRE(s.energyBar == SideDataRect(471, 11, 592, 13));
            REQUIRE(s.energyNum == SideDataRect(529, 18, 529, 18));
            REQUIRE(s.energyMax == SideDataRect(595, 1, 595, 1));
            REQUIRE(s.energy0 == SideDataRect(468, 1, 468, 1));
            REQUIRE(s.metalBar == SideDataRect(218, 11, 339, 13));
            REQUIRE(s.metalNum == SideDataRect(278, 18, 278, 18));
            REQUIRE(s.metalMax == SideDataRect(341, 1, 341, 1));
            REQUIRE(s.metal0 == SideDataRect(215, 1, 215, 1));
            REQUIRE(s.totalUnits == SideDataRect(560, 7, 583, 15));
            REQUIRE(s.totalTime == SideDataRect(605, 7, 628, 15));
            REQUIRE(s.energyProduced == SideDataRect(609, 5, 609, 5));
            REQUIRE(s.energyConsumed == SideDataRect(609, 17, 609, 17));
            REQUIRE(s.metalProduced == SideDataRect(358, 5, 358, 5));
            REQUIRE(s.metalConsumed == SideDataRect(358, 17, 358, 17));
            REQUIRE(s.logo2 == SideDataRect(132, 455, 152, 475));
            REQUIRE(s.unitName == SideDataRect(245, 452, 245, 460));
            REQUIRE(s.damageBar == SideDataRect(200, 463, 290, 465));
            REQUIRE(s.unitMetalMake == SideDataRect(350, 458, 170, 478));
            REQUIRE(s.unitMetalUse == SideDataRect(350, 468, 170, 478));
            REQUIRE(s.unitEnergyMake == SideDataRect(400, 458, 170, 468));
            REQUIRE(s.unitEnergyUse == SideDataRect(400, 468, 170, 468));
            REQUIRE(s.missionText == SideDataRect(385, 449, 400, 450));
            REQUIRE(s.unitName2 == SideDataRect(555, 452, 555, 460));
            REQUIRE(s.damageBar2 == SideDataRect(510, 463, 600, 465));
            REQUIRE(s._name == SideDataRect(132, 452, 142, 460));
            REQUIRE(s.description == SideDataRect(132, 465, 142, 472));
            REQUIRE(s.reload1 == SideDataRect(132, 450, 148, 458));
            REQUIRE(s.reload2 == SideDataRect(132, 460, 148, 468));
            REQUIRE(s.reload3 == SideDataRect(132, 470, 148, 478));
        }
    }
}
