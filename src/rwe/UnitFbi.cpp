#include "UnitFbi.h"

namespace rwe
{
    UnitFbi parseUnitFbi(const TdfBlock& tdf)
    {
        auto infoBlock = tdf.findBlock("UNITINFO");
        if (!infoBlock)
        {
            throw std::runtime_error("FBI missing UNITINFO block");
        }

        return parseUnitInfoBlock(*infoBlock);
    }

    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf)
    {
        UnitFbi u;

        u.unitName = tdf.expectString("UnitName");
        u.objectName = tdf.expectString("Objectname");
        u.soundCategory = tdf.expectString("SoundCategory");

        const std::string emptyString;
        u.movementClass = tdf.findValue("MovementClass").get_value_or(emptyString);

        u.turnRate = tdf.extractFloat("TurnRate").get_value_or(0.0f);
        u.maxVelocity = tdf.extractFloat("MaxVelocity").get_value_or(0.0f);
        u.acceleration = tdf.extractFloat("Acceleration").get_value_or(0.0f);
        u.brakeRate = tdf.extractFloat("BrakeRate").get_value_or(0.0f);

        u.footprintX = tdf.extractUint("FootprintX").get_value_or(0);
        u.footprintZ = tdf.extractUint("FootprintZ").get_value_or(0);

        u.maxSlope = tdf.extractUint("MaxSlope").get_value_or(0);
        u.minWaterDepth = tdf.extractUint("MinWaterDepth").get_value_or(0);
        u.maxWaterDepth = tdf.extractUint("MaxWaterDepth").get_value_or(0);

        return u;
    }
}
