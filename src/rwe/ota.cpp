#include "ota.h"

#include <rwe/tdf.h>

namespace rwe
{
    OtaParseException::OtaParseException(const std::string& __arg) : runtime_error(__arg)
    {
    }

    OtaParseException::OtaParseException(const char* string) : runtime_error(string)
    {
    }

    OtaRecord parseOta(const TdfBlock& tdf)
    {
        auto block = tdf.findBlock("GlobalHeader");
        if (!block)
        {
            throw OtaParseException("Could not find GlobalHeader block");
        }

        return parseOtaGlobalHeader(*block);
    }

    OtaRecord parseOtaGlobalHeader(const TdfBlock& tdf)
    {
        std::string emptyString;

        OtaRecord r;

        r.missionName = tdf.expectString("missionname");

        r.missionDescription = tdf.findValue("missiondescription").get_value_or(emptyString);
        r.planet = tdf.findValue("planet").get_value_or(emptyString);
        r.missionHint = tdf.findValue("missionhint").get_value_or(emptyString);
        r.brief = tdf.findValue("brief").get_value_or(emptyString);
        r.narration = tdf.findValue("narration").get_value_or(emptyString);
        r.glamour = tdf.findValue("glamour").get_value_or(emptyString);
        r.lineOfSight = tdf.extractInt("lineofsight").get_value_or(0);
        r.mapping = tdf.extractInt("mapping").get_value_or(0);
        r.tidalStrength = tdf.extractInt("tidalstrength").get_value_or(0);
        r.solarStrength = tdf.extractInt("solarstrength").get_value_or(0);
        r.lavaWorld = tdf.extractBool("lavaworld").get_value_or(false);
        r.killMul = tdf.extractInt("killmul").get_value_or(50);
        r.timeMul = tdf.extractInt("timemul").get_value_or(0);
        r.minWindSpeed = tdf.extractInt("minwindspeed").get_value_or(0);
        r.maxWindSpeed = tdf.extractInt("maxwindspeed").get_value_or(0);
        r.gravity = tdf.extractInt("gravity").get_value_or(112);
        r.numPlayers = tdf.findValue("numplayers").get_value_or(emptyString);
        r.size = tdf.findValue("size").get_value_or(emptyString);
        r.memory = tdf.findValue("memory").get_value_or(emptyString);
        r.useOnlyUnits = tdf.findValue("useonlyunits").get_value_or(emptyString);

        r.schemaCount = tdf.expectInt("SCHEMACOUNT");

        for (int i = 0; i < r.schemaCount; ++i)
        {
            auto schemaName = std::string("Schema ") + std::to_string(i);
            auto schemaBlock = tdf.findBlock(schemaName);
            if (!schemaBlock)
            {
                throw OtaParseException("Missing block: " + schemaName);
            }

            r.schemas.push_back(parseOtaSchema(*schemaBlock));
        }

        return r;
    }

    OtaSchema parseOtaSchema(const TdfBlock& tdf)
    {
        std::string emptyString;
        std::string defaultAiProfile("DEFAULT");

        OtaSchema s;
        s.type = tdf.expectString("Type");
        s.aiProfile = tdf.findValue("aiprofile").get_value_or(defaultAiProfile);
        s.surfaceMetal = tdf.extractInt("SurfaceMetal").get_value_or(3);
        s.mohoMetal = tdf.extractInt("MohoMetal").get_value_or(30);
        s.humanMetal = tdf.extractInt("HumanMetal").get_value_or(1000);
        s.computerMetal = tdf.extractInt("ComputerMetal").get_value_or(1000);
        s.humanEnergy = tdf.extractInt("HumanEnergy").get_value_or(1000);
        s.computerEnergy = tdf.extractInt("ComputerEnergy").get_value_or(1000);
        s.meteorWeapon = tdf.findValue("MeteorWeapon").get_value_or(emptyString);
        s.meteorRadius = tdf.extractInt("MeteorRadius").get_value_or(0);
        s.meteorDensity = tdf.extractFloat("MeteorDensity").get_value_or(0.0f);
        s.meteorDuration = tdf.extractInt("MeteorDuration").get_value_or(0);
        s.meteorInterval = tdf.extractInt("MeteorInterval").get_value_or(0);

        auto featuresBlock = tdf.findBlock("features");
        if (featuresBlock)
        {
            for (const auto& e : featuresBlock->entries)
            {
                auto block = boost::get<TdfBlock>(e.value.get());
                if (block != nullptr)
                {
                    s.features.push_back(parseOtaFeature(*block));
                }
            }
        }

        auto specialsBlock = tdf.findBlock("specials");
        if (specialsBlock)
        {
            for (const auto& e : specialsBlock->entries)
            {
                auto block = boost::get<TdfBlock>(e.value.get());
                if (block != nullptr)
                {
                    s.specials.push_back(parseOtaSpecial(*block));
                }
            }
        }

        return s;
    }

    OtaFeature parseOtaFeature(const TdfBlock& tdf)
    {
        OtaFeature f;
        f.featureName = tdf.expectString("Featurename");
        f.xPos = tdf.expectInt("XPos");
        f.zPos = tdf.expectInt("ZPos");
        return f;
    }

    OtaSpecial parseOtaSpecial(const TdfBlock& tdf)
    {
        OtaSpecial s;
        s.specialWhat = tdf.expectString("specialwhat");
        s.xPos = tdf.expectInt("XPos");
        s.zPos = tdf.expectInt("ZPos");
        return s;
    }
}
