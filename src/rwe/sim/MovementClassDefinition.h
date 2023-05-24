#pragma once

#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct MovementClassDefinition
    {
        std::string name;
        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
    };

    MovementClassDefinition parseMovementClass(const TdfBlock& block);

    std::vector<std::pair<std::string, MovementClassDefinition>> parseMovementTdf(const TdfBlock& root);
}
