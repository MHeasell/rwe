#pragma once

#include <string>
namespace rwe
{
    struct FeatureMediaInfo
    {
        std::string world;
        std::string description;
        std::string category;

        bool animating;
        std::string fileName;
        std::string seqName;
        bool animTrans;
        std::string seqNameShad;
        bool shadTrans;

        std::string object;

        std::string seqNameReclamate;

        std::string seqNameBurn;
        std::string seqNameBurnShad;

        std::string seqNameDie;
    };
}
