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

        tdf.read("UnitName", u.unitName);
        tdf.read("Objectname", u.objectName);
        tdf.read("SoundCategory", u.soundCategory);

        tdf.readOrDefault("MovementClass", u.movementClass);

        tdf.readOrDefault("TurnRate", u.turnRate);
        tdf.readOrDefault("MaxVelocity", u.maxVelocity);
        tdf.readOrDefault("Acceleration", u.acceleration);
        tdf.readOrDefault("BrakeRate", u.brakeRate);

        tdf.readOrDefault("FootprintX", u.footprintX);
        tdf.readOrDefault("FootprintZ", u.footprintZ);

        tdf.readOrDefault("MaxSlope", u.maxSlope);
        tdf.readOrDefault("MaxWaterSlope", u.maxWaterSlope, u.maxSlope);
        tdf.readOrDefault("MinWaterDepth", u.minWaterDepth);
        tdf.readOrDefault("MaxWaterDepth", u.maxWaterDepth);

        tdf.readOrDefault("CanAttack", u.canAttack);

        tdf.readOrDefault("Commander", u.commander);

        tdf.readOrDefault("MaxDamage", u.maxDamage);

        tdf.readOrDefault("BMCode", u.bmCode);

        tdf.readOrDefault("Weapon1", u.weapon1);
        tdf.readOrDefault("Weapon2", u.weapon2);
        tdf.readOrDefault("Weapon3", u.weapon3);

        tdf.readOrDefault("ExplodeAs", u.explodeAs);

        tdf.readOrDefault("Builder", u.builder);

        tdf.readOrDefault("BuildTime", u.buildTime);

        return u;
    }
}
