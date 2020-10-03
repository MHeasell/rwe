#pragma once

#include <rwe/Energy.h>
#include <rwe/Metal.h>
#include <rwe/fbi/FbiAnglePerTick.h>
#include <rwe/fbi/FbiDistancePerTick.h>
#include <rwe/fbi/FbiDistancePerTickSquared.h>
#include <string>

namespace rwe
{
    struct UnitFbi
    {
        std::string unitName;
        std::string objectName;
        std::string soundCategory;
        std::string movementClass;

        std::string name;

        FbiAnglePerTick turnRate;
        FbiDistancePerTick maxVelocity;
        FbiDistancePerTickSquared acceleration;
        FbiDistancePerTickSquared brakeRate;

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

        bool hideDamage;
        bool showPlayerName;

        std::string yardMap;
    };
}
