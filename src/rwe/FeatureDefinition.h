#ifndef RWE_FEATUREDEFINITION_H
#define RWE_FEATUREDEFINITION_H

#include <boost/optional.hpp>
#include <string>

namespace rwe
{
    struct FeatureDefinition
    {
        std::string world;
        std::string description;
        std::string cetegory;
        bool animating;
        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int height;
        std::string fileName;
        std::string seqName;
        boost::optional<std::string> seqNameShad;
        boost::optional<std::string> seqNameReclamate;
        boost::optional<std::string> seqNameDie;
        boost::optional<std::string> seqNameBurn;
        boost::optional<std::string> seqNameBurnShad;
        boost::optional<std::string> featureBurnt;
        boost::optional<std::string> featureReclamate;
        boost::optional<std::string> featureDead;

        boost::optional<unsigned int> burnMin;
        boost::optional<unsigned int> burnMax;
        boost::optional<unsigned int> sparkTime;
        boost::optional<unsigned int> spreadChance;
        boost::optional<std::string> burnWeapon;

        boost::optional<bool> animTrans;
        boost::optional<bool> shadTrans;

        bool reclaimable;
        bool flamable;
        bool geothermal;
        unsigned int hitDensity;

        bool reproduce;
        unsigned int reproduceArea;

        bool noDisplayInfo;
        bool permanent;

        boost::optional<bool> blocking;

        boost::optional<unsigned int> energy;
        boost::optional<unsigned int> metal;

        boost::optional<unsigned int> damage;
        bool indestructible;
    };
}

#endif
