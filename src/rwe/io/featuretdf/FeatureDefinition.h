#pragma once

#include <optional>
#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct FeatureDefinition
    {
        std::string world;
        std::string description;
        std::string category;

        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int height;

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
        unsigned int metal;
        unsigned int energy;

        bool flamable;
        std::string seqNameBurn;
        std::string seqNameBurnShad;
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
        std::string seqNameDie;
        std::string featureDead;
    };
}
