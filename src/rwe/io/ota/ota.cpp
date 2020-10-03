#include "ota.h"

#include <rwe/io/tdf/tdf.h>

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

        tdf.read("missionname", r.missionName);

        tdf.readOrDefault("missiondescription", r.missionDescription);
        tdf.readOrDefault("planet", r.planet);
        tdf.readOrDefault("missionhint", r.missionHint);
        tdf.readOrDefault("brief", r.brief);
        tdf.readOrDefault("narration", r.narration);
        tdf.readOrDefault("glamour", r.glamour);
        tdf.readOrDefault("lineofsight", r.lineOfSight);
        tdf.readOrDefault("mapping", r.mapping);
        tdf.readOrDefault("tidalstrength", r.tidalStrength);
        tdf.readOrDefault("solarstrength", r.solarStrength);
        tdf.readOrDefault("lavaworld", r.lavaWorld);
        tdf.readOrDefault("killmul", r.killMul, 50);
        tdf.readOrDefault("timemul", r.timeMul);
        tdf.readOrDefault("minwindspeed", r.minWindSpeed);
        tdf.readOrDefault("maxwindspeed", r.maxWindSpeed);
        tdf.readOrDefault("gravity", r.gravity, 112);
        tdf.readOrDefault("numplayers", r.numPlayers);
        tdf.readOrDefault("size", r.size);
        tdf.readOrDefault("memory", r.memory);
        tdf.readOrDefault("useonlyunits", r.useOnlyUnits);

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
        tdf.read("Type", s.type);
        tdf.readOrDefault("aiprofile", s.aiProfile, std::string("DEFAULT"));
        tdf.readOrDefault("SurfaceMetal", s.surfaceMetal, 3);
        tdf.readOrDefault("MohoMetal", s.mohoMetal, 30);
        tdf.readOrDefault("HumanMetal", s.humanMetal, 1000);
        tdf.readOrDefault("ComputerMetal", s.computerMetal, 1000);
        tdf.readOrDefault("HumanEnergy", s.humanEnergy, 1000);
        tdf.readOrDefault("ComputerEnergy", s.computerEnergy, 1000);
        tdf.readOrDefault("MeteorWeapon", s.meteorWeapon);
        tdf.readOrDefault("MeteorRadius", s.meteorRadius);
        tdf.readOrDefault("MeteorDensity", s.meteorDensity);
        tdf.readOrDefault("MeteorDuration", s.meteorDuration);
        tdf.readOrDefault("MeteorInterval", s.meteorInterval);

        auto featuresBlockOption = tdf.findBlock("features");
        if (featuresBlockOption)
        {
            auto featuresBlock = &featuresBlockOption->get();
            int i = 0;
            auto block = featuresBlock->findBlock("feature" + std::to_string(i));
            while (block)
            {
                s.features.push_back(parseOtaFeature(*block));

                i += 1;
                block = featuresBlock->findBlock("feature" + std::to_string(i));
            }
        }

        auto specialsBlockOption = tdf.findBlock("specials");
        if (specialsBlockOption)
        {
            auto specialsBlock = &specialsBlockOption->get();
            int i = 0;
            auto block = specialsBlock->findBlock("special" + std::to_string(i));
            while (block)
            {
                s.specials.push_back(parseOtaSpecial(*block));

                i += 1;
                block = specialsBlock->findBlock("special" + std::to_string(i));
            }
        }

        return s;
    }

    OtaFeature parseOtaFeature(const TdfBlock& tdf)
    {
        OtaFeature f;
        tdf.read("Featurename", f.featureName);
        tdf.read("XPos", f.xPos);
        tdf.read("ZPos", f.zPos);
        return f;
    }

    OtaSpecial parseOtaSpecial(const TdfBlock& tdf)
    {
        OtaSpecial s;
        tdf.read("specialwhat", s.specialWhat);
        tdf.read("XPos", s.xPos);
        tdf.read("ZPos", s.zPos);
        return s;
    }
}
