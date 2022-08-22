#pragma once

#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/sim/SimScalar.h>
#include <string>

namespace rwe
{
    struct UnitDefinition
    {
        std::string unitName;
        std::string objectName;
        std::string movementClass;

        SimScalar turnRate;
        SimScalar maxVelocity;
        SimScalar acceleration;
        SimScalar brakeRate;

        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;

        bool canAttack;
        bool canMove;
        bool canGuard;

        bool commander;

        unsigned int maxDamage;

        bool bmCode;

        bool floater;
        bool canHover;

        std::string weapon1;
        std::string weapon2;
        std::string weapon3;

        std::string explodeAs;

        bool builder;
        unsigned int buildTime;
        Energy buildCostEnergy;
        Metal buildCostMetal;

        unsigned int workerTime;

        unsigned int buildDistance;

        bool onOffable;
        bool activateWhenBuilt;

        Energy energyMake;
        Metal metalMake;
        Energy energyUse;
        Metal metalUse;

        Metal makesMetal;
        Metal extractsMetal;

        std::string yardMap;

        std::string corpse;
    };
}
