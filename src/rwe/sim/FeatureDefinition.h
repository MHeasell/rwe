#pragma once

#include <rwe/sim/SimScalar.h>
#include <string>

namespace rwe
{
    struct FeatureDefinition
    {
        int footprintX;
        int footprintZ;
        SimScalar height;

        bool reclaimable;
        bool autoreclaimable;
        std::string featureReclamate;
        unsigned int metal;
        unsigned int energy;

        bool flamable;
        std::string featureBurnt;
        unsigned int burnMin;
        unsigned int burnMax;
        unsigned int sparkTime;
        unsigned int spreadChance;
        std::string burnWeapon;

        bool geothermal;

        unsigned int hitDensity;

        bool reproduce;
        unsigned int reproduceArea;

        bool noDisplayInfo;

        bool permanent;

        bool blocking;

        bool indestructible;
        unsigned int damage;
        std::string featureDead;

        bool isStanding() const;
    };
}
