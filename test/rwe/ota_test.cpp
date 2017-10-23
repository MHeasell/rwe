#include <catch.hpp>

#include <rwe/ota.h>
#include <rwe/tdf.h>

namespace rwe
{
    TEST_CASE("parseOta")
    {
        SECTION("parses ota files")
        {

            std::string input = R"TDF(
[GlobalHeader]
	{
	missionname=Painted Desert;
	missiondescription=18 X 18  Battle in the Many Mesas area.;
	planet=Desert;
	missionhint=;
	brief=;
	narration=;
	glamour=;
	lineofsight=0;
	mapping=0;
	tidalstrength=0;
	solarstrength=35;
	lavaworld=0;
	killmul=50;
	timemul=0;
	minwindspeed=100;
	maxwindspeed=4000;
	gravity=112;
	numplayers=2, 3, 4, 6;
	size=19 x 19;
	memory=32 mb;
	useonlyunits=Painted Desert.tdf;
	SCHEMACOUNT=2;
	[Schema 0]
		{
		Type=Network 2;
		aiprofile=RADAI;
		SurfaceMetal=3;
		MohoMetal=40;
		HumanMetal=1000;
		ComputerMetal=2000;
		HumanEnergy=3000;
		ComputerEnergy=4000;
		MeteorWeapon=radMETEOR;
		MeteorRadius=1;
		MeteorDensity=.2;
		MeteorDuration=3;
		MeteorInterval=4;
		[specials]
			{
			[special0]
				{
				specialwhat=StartPos1;
				XPos=1824;
				ZPos=1008;
				}
			[special1]
				{
				specialwhat=StartPos2;
				XPos=7872;
				ZPos=8432;
				}
			}
		}
	[Schema 1]
		{
		Type=Network 3;
		aiprofile=;
		SurfaceMetal=5;
		MohoMetal=60;
		HumanMetal=1000;
		ComputerMetal=1000;
		HumanEnergy=1000;
		ComputerEnergy=1000;
		MeteorWeapon=;
		MeteorRadius=0;
		MeteorDensity=0;
		MeteorDuration=0;
		MeteorInterval=0;
		[features]
			{
			[feature0]
				{
				Featurename=DryMetal03;
				XPos=121;
				ZPos=495;
				}
			[feature1]
				{
				Featurename=DryMetal01;
				XPos=482;
				ZPos=455;
				}
			[feature2]
				{
				Featurename=DryMetal02;
				XPos=460;
				ZPos=111;
				}
			[feature3]
				{
				Featurename=DryMetal03;
				XPos=119;
				ZPos=108;
				}
			}
		[specials]
			{
			[special0]
				{
				specialwhat=StartPos1;
				XPos=1776;
				ZPos=1536;
				}
			[special1]
				{
				specialwhat=StartPos2;
				XPos=7488;
				ZPos=2016;
				}
			[special2]
				{
				specialwhat=StartPos3;
				XPos=7632;
				ZPos=7408;
				}
			[special3]
				{
				specialwhat=StartPos4;
				XPos=1856;
				ZPos=8096;
				}
			}
		}
	}
)TDF";

            OtaSchema schema0;
            schema0.type = "Network 2";
            schema0.aiProfile = "RADAI";
            schema0.surfaceMetal = 3;
            schema0.mohoMetal = 40;
            schema0.humanMetal = 1000;
            schema0.computerMetal = 2000;
            schema0.humanEnergy = 3000;
            schema0.computerEnergy = 4000;
            schema0.meteorWeapon = "radMETEOR";
            schema0.meteorRadius = 1;
            schema0.meteorDensity = 0.2f;
            schema0.meteorDuration = 3;
            schema0.meteorInterval = 4;
            schema0.specials = std::vector<OtaSpecial>{
                {"StartPos1", 1824, 1008},
                {"StartPos2", 7872, 8432},
            };
            schema0.features = std::vector<OtaFeature>();

            OtaSchema schema1;
            schema1.type = "Network 3";
            schema1.aiProfile = "";
            schema1.surfaceMetal = 5;
            schema1.mohoMetal = 60;
            schema1.humanMetal = 1000;
            schema1.computerMetal = 1000;
            schema1.humanEnergy = 1000;
            schema1.computerEnergy = 1000;
            schema1.meteorWeapon = "";
            schema1.meteorRadius = 0;
            schema1.meteorDensity = 0;
            schema1.meteorDuration = 0;
            schema1.meteorInterval = 0;
            schema1.features = std::vector<OtaFeature>{
                {"DryMetal03", 121, 495},
                {"DryMetal01", 482, 455},
                {"DryMetal02", 460, 111},
                {"DryMetal03", 119, 108},
            };
            schema1.specials = std::vector<OtaSpecial>{
                {"StartPos1", 1776, 1536},
                {"StartPos2", 7488, 2016},
                {"StartPos3", 7632, 7408},
                {"StartPos4", 1856, 8096},
            };

            OtaRecord ota;
            ota.missionName = "Painted Desert";
            ota.missionDescription = "18 X 18  Battle in the Many Mesas area.";
            ota.planet = "Desert";
            ota.missionHint = "";
            ota.brief = "";
            ota.narration = "";
            ota.glamour = "";
            ota.lineOfSight = 0;
            ota.mapping = 0;
            ota.tidalStrength = 0;
            ota.solarStrength = 35;
            ota.lavaWorld = false;
            ota.killMul = 50;
            ota.timeMul = 0;
            ota.minWindSpeed = 100;
            ota.maxWindSpeed = 4000;
            ota.gravity = 112;
            ota.numPlayers = "2, 3, 4, 6";
            ota.size = "19 x 19";
            ota.memory = "32 mb";
            ota.useOnlyUnits = "Painted Desert.tdf";
            ota.schemaCount = 2;
            ota.schemas.push_back(schema0);
            ota.schemas.push_back(schema1);

            auto parsedRecord = parseOta(parseTdfFromString(input));


            REQUIRE(parsedRecord.missionName == ota.missionName);
            REQUIRE(parsedRecord.missionDescription == ota.missionDescription);
            REQUIRE(parsedRecord.planet == ota.planet);
            REQUIRE(parsedRecord.missionHint == ota.missionHint);
            REQUIRE(parsedRecord.brief == ota.brief);
            REQUIRE(parsedRecord.narration == ota.narration);
            REQUIRE(parsedRecord.glamour == ota.glamour);
            REQUIRE(parsedRecord.lineOfSight == ota.lineOfSight);
            REQUIRE(parsedRecord.mapping == ota.mapping);
            REQUIRE(parsedRecord.tidalStrength == ota.tidalStrength);
            REQUIRE(parsedRecord.solarStrength == ota.solarStrength);
            REQUIRE(parsedRecord.lavaWorld == ota.lavaWorld);
            REQUIRE(parsedRecord.killMul == ota.killMul);
            REQUIRE(parsedRecord.timeMul == ota.timeMul);
            REQUIRE(parsedRecord.minWindSpeed == ota.minWindSpeed);
            REQUIRE(parsedRecord.maxWindSpeed == ota.maxWindSpeed);
            REQUIRE(parsedRecord.gravity == ota.gravity);
            REQUIRE(parsedRecord.numPlayers == ota.numPlayers);
            REQUIRE(parsedRecord.size == ota.size);
            REQUIRE(parsedRecord.memory == ota.memory);
            REQUIRE(parsedRecord.useOnlyUnits == ota.useOnlyUnits);
            REQUIRE(parsedRecord.schemaCount == ota.schemaCount);

            REQUIRE(parsedRecord.schemas.size() == ota.schemas.size());

            for (unsigned int i = 0; i < parsedRecord.schemas.size(); ++i)
            {
                const auto& a = parsedRecord.schemas[i];
                const auto& b = ota.schemas[i];

                REQUIRE(a.type == b.type);
                REQUIRE(a.aiProfile == b.aiProfile);
                REQUIRE(a.surfaceMetal == b.surfaceMetal);
                REQUIRE(a.mohoMetal == b.mohoMetal);
                REQUIRE(a.humanMetal == b.humanMetal);
                REQUIRE(a.computerMetal == b.computerMetal);
                REQUIRE(a.humanEnergy == b.humanEnergy);
                REQUIRE(a.computerEnergy == b.computerEnergy);
                REQUIRE(a.meteorWeapon == b.meteorWeapon);
                REQUIRE(a.meteorRadius == b.meteorRadius);
                REQUIRE(a.meteorDensity == b.meteorDensity);
                REQUIRE(a.meteorDuration == b.meteorDuration);
                REQUIRE(a.meteorInterval == b.meteorInterval);

                REQUIRE(a.specials.size() == b.specials.size());
                for (unsigned int j = 0; j < a.specials.size(); ++j)
                {
                    const auto& sa = a.specials[j];
                    const auto& sb = b.specials[j];

                    REQUIRE(sa.specialWhat == sb.specialWhat);
                    REQUIRE(sa.xPos == sb.xPos);
                    REQUIRE(sa.zPos == sb.zPos);
                }

                REQUIRE(a.features.size() == b.features.size());
                for (unsigned int j = 0; j < a.features.size(); ++j)
                {
                    const auto& fa = a.features[j];
                    const auto& fb = b.features[j];

                    REQUIRE(fa.featureName == fb.featureName);
                    REQUIRE(fa.xPos == fb.xPos);
                    REQUIRE(fa.zPos == fb.zPos);
                }
            }
        }
    }
}
