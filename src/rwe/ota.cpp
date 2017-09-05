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

        r.missionName = expectString(tdf, "missionname");
        r.missionDescription = expectString(tdf, "missiondescription");
        r.planet = expectString(tdf, "planet");
        r.missionHint = expectString(tdf, "missionhint");
        r.brief = expectString(tdf, "brief");
        r.narration = expectString(tdf, "narration");
        r.glamour = expectString(tdf, "glamour");
        r.lineOfSight = expectInt(tdf, "lineofsight");
        r.mapping = expectInt(tdf, "mapping");
        r.tidalStrength = expectInt(tdf, "tidalstrength");
        r.solarStrength = expectInt(tdf, "solarstrength");
        r.lavaWorld = extractBool(tdf, "lavaworld").get_value_or(false);
        r.killMul = expectInt(tdf, "killmul");
        r.timeMul = expectInt(tdf, "timemul");
        r.minWindSpeed = expectInt(tdf, "minwindspeed");
        r.maxWindSpeed = expectInt(tdf, "maxwindspeed");
        r.gravity = expectInt(tdf, "gravity");
        r.numPlayers = expectString(tdf, "numplayers");
        r.size = expectString(tdf, "size");
        r.memory = expectString(tdf, "memory");
        r.useOnlyUnits = expectString(tdf, "useonlyunits");
        r.schemaCount = expectInt(tdf, "SCHEMACOUNT");

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
        s.type = expectString(tdf, "Type");
        s.aiProfile = expectString(tdf, "aiprofile");
        s.surfaceMetal = expectInt(tdf, "SurfaceMetal");
        s.mohoMetal = expectInt(tdf, "MohoMetal");
        s.humanMetal = expectInt(tdf, "HumanMetal");
        s.computerMetal = expectInt(tdf, "ComputerMetal");
        s.humanEnergy = expectInt(tdf, "HumanEnergy");
        s.computerEnergy = expectInt(tdf, "ComputerEnergy");
        s.meteorWeapon = expectString(tdf, "MeteorWeapon");
        s.meteorRadius = expectInt(tdf, "MeteorRadius");
        s.meteorDensity = expectFloat(tdf, "MeteorDensity");
        s.meteorDuration = expectInt(tdf, "MeteorDuration");
        s.meteorInterval = expectInt(tdf, "MeteorInterval");

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
        f.featureName = expectString(tdf, "Featurename");
        f.xPos = expectInt(tdf, "XPos");
        f.zPos = expectInt(tdf, "ZPos");
        return f;
    }

    OtaSpecial parseOtaSpecial(const TdfBlock& tdf)
    {
        OtaSpecial s;
        s.specialWhat = expectString(tdf, "specialwhat");
        s.xPos = expectInt(tdf, "XPos");
        s.zPos = expectInt(tdf, "ZPos");
        return s;
    }
}
