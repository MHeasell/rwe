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
        OtaRecord r;

        r.missionName = tdf.expectString("missionname");
        r.missionDescription = tdf.expectString("missiondescription");
        r.planet = tdf.expectString("planet");
        r.missionHint = tdf.expectString("missionhint");
        r.brief = tdf.expectString("brief");
        r.narration = tdf.expectString("narration");
        r.glamour = tdf.expectString("glamour");
        r.lineOfSight = tdf.expectInt("lineofsight");
        r.mapping = tdf.expectInt("mapping");
        r.tidalStrength = tdf.expectInt("tidalstrength");
        r.solarStrength = tdf.expectInt("solarstrength");
        r.lavaWorld = tdf.extractBool("lavaworld").get_value_or(false);
        r.killMul = tdf.expectInt("killmul");
        r.timeMul = tdf.expectInt("timemul");
        r.minWindSpeed = tdf.expectInt("minwindspeed");
        r.maxWindSpeed = tdf.expectInt("maxwindspeed");
        r.gravity = tdf.expectInt("gravity");
        r.numPlayers = tdf.expectString("numplayers");
        r.size = tdf.expectString("size");
        r.memory = tdf.expectString("memory");
        r.useOnlyUnits = tdf.expectString("useonlyunits");
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
        OtaSchema s;
        s.type = tdf.expectString("Type");
        s.aiProfile = tdf.expectString("aiprofile");
        s.surfaceMetal = tdf.expectInt("SurfaceMetal");
        s.mohoMetal = tdf.expectInt("MohoMetal");
        s.humanMetal = tdf.expectInt("HumanMetal");
        s.computerMetal = tdf.expectInt("ComputerMetal");
        s.humanEnergy = tdf.expectInt("HumanEnergy");
        s.computerEnergy = tdf.expectInt("ComputerEnergy");
        s.meteorWeapon = tdf.expectString("MeteorWeapon");
        s.meteorRadius = tdf.expectInt("MeteorRadius");
        s.meteorDensity = tdf.expectFloat("MeteorDensity");
        s.meteorDuration = tdf.expectInt("MeteorDuration");
        s.meteorInterval = tdf.expectInt("MeteorInterval");

        auto featuresBlock = tdf.findBlock("features");
        if (featuresBlock)
        {
            unsigned int i = 0;
            auto featureBlock = featuresBlock->findBlock("feature" + std::to_string(i));
            while (featureBlock)
            {
                s.features.push_back(parseOtaFeature(*featureBlock));
                ++i;
                featureBlock = featuresBlock->findBlock("feature" + std::to_string(i));
            }
        }

        auto specialsBlock = tdf.findBlock("specials");
        if (specialsBlock)
        {
            unsigned int i = 0;
            auto specialBlock = specialsBlock->findBlock("special" + std::to_string(i));
            while (specialBlock)
            {
                s.specials.push_back(parseOtaSpecial(*specialBlock));
                ++i;
                specialBlock = specialsBlock->findBlock("special" + std::to_string(i));
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
