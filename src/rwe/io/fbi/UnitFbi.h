#pragma once

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
        std::string description;

        unsigned int turnRate;
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
        bool canMove;
        bool canGuard;

        bool commander;

        unsigned int maxDamage;

        bool bmCode;

        bool floater;
        bool canHover;

        bool canFly;

        unsigned int cruiseAlt;

        std::string weapon1;
        std::string weapon2;
        std::string weapon3;

        std::string explodeAs;

        bool builder;
        unsigned int buildTime;
        unsigned int buildCostEnergy;
        unsigned int buildCostMetal;

        unsigned int workerTime;

        unsigned int buildDistance;

        bool onOffable;
        bool activateWhenBuilt;

        float energyMake;
        float metalMake;
        float energyUse;
        float metalUse;

        float makesMetal;
        float extractsMetal;

        unsigned int energyStorage;
        unsigned int metalStorage;

        unsigned int windGenerator;

        bool hideDamage;
        bool showPlayerName;

        std::string yardMap;

        std::string corpse;
    };
}
