#pragma once

#include <rwe/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct MovementClass
    {
        std::string name;
        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
    };

    MovementClass parseMovementClass(const TdfBlock& block);

    std::vector<std::pair<std::string, MovementClass>> parseMovementTdf(const TdfBlock& root);
}
