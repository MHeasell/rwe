#ifndef RWE_UNITFBI_H
#define RWE_UNITFBI_H

#include <rwe/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct UnitFbi
    {
        std::string unitName;
        std::string objectName;
        std::string soundCategory;
        std::string movementClass;

        float turnRate;
        float maxVelocity;
        float acceleration;
        float brakeRate;

        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;

        bool canAttack;

        unsigned int maxDamage;

        bool bmCode;

        std::string weapon1;
        std::string weapon2;
        std::string weapon3;

        std::string explodeAs;
    };

    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf);

    UnitFbi parseUnitFbi(const TdfBlock& tdf);
}

#endif
