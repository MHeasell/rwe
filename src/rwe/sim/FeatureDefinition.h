#pragma once

#include <optional>
#include <rwe/sim/FeatureDefinitionId.h>
#include <rwe/sim/SimScalar.h>
#include <string>

namespace rwe
{
    struct FeatureDefinition
    {
        std::string name;

        int footprintX;
        int footprintZ;
        SimScalar height;

        bool reclaimable;
        bool autoreclaimable;
        std::optional<FeatureDefinitionId> featureReclamate;
        unsigned int metal;
        unsigned int energy;

        bool flamable;
        std::optional<FeatureDefinitionId> featureBurnt;
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
        std::optional<FeatureDefinitionId> featureDead;

        bool isStanding() const;
    };
}
