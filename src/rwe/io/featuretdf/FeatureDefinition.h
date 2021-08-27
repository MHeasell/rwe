#pragma once

#include <optional>
#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct FeatureDefinition
    {
        static FeatureDefinition fromTdf(const TdfBlock& tdf);

        std::string world;
        std::string description;
        std::string category;

        int footprintX;
        int footprintZ;
        int height;

        bool animating;
        std::string fileName;
        std::string seqName;
        bool animTrans;
        std::string seqNameShad;
        bool shadTrans;

        std::string object;

        bool reclaimable;
        bool autoreclaimable;
        std::string seqNameReclamate;
        std::string featureReclamate;
        int metal;
        int energy;

        bool flamable;
        std::string seqNameBurn;
        std::string seqNameBurnShad;
        std::string featureBurnt;
        int burnMin;
        int burnMax;
        int sparkTime;
        int spreadChance;
        std::string burnWeapon;

        bool geothermal;

        int hitDensity;

        bool reproduce;
        int reproduceArea;

        bool noDisplayInfo;

        bool permanent;

        bool blocking;

        bool indestructible;
        int damage;
        std::string seqNameDie;
        std::string featureDead;
    };
}
