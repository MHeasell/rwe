#pragma once

#include <rwe/io/fbi/FbiAnglePerTick.h>
#include <rwe/io/fbi/FbiDistancePerTick.h>
#include <rwe/io/fbi/FbiDistancePerTickSquared.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
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

        int footprintX;
        int footprintZ;
        int maxSlope;
        int maxWaterSlope;
        int minWaterDepth;
        int maxWaterDepth;

        bool canAttack;
        bool canMove;
        bool canGuard;

        bool commander;

        int maxDamage;

        bool bmCode;

        bool floater;
        bool canHover;

        std::string weapon1;
        std::string weapon2;
        std::string weapon3;

        std::string explodeAs;

        bool builder;
        int buildTime;
        Energy buildCostEnergy;
        Metal buildCostMetal;

        int workerTime;

        int buildDistance;

        bool onOffable;
        bool activateWhenBuilt;

        Energy energyMake;
        Metal metalMake;
        Energy energyUse;
        Metal metalUse;

        Metal makesMetal;
        Metal extractsMetal;

        Energy energyStorage;
        Metal metalStorage;

        bool hideDamage;
        bool showPlayerName;

        std::string yardMap;
    };
}
