#pragma once

#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct MovementClass
    {
        std::string name;
        int footprintX;
        int footprintZ;
        int minWaterDepth;
        int maxWaterDepth;
        int maxSlope;
        int maxWaterSlope;
    };

    MovementClass parseMovementClass(const TdfBlock& block);

    std::vector<std::pair<std::string, MovementClass>> parseMovementTdf(const TdfBlock& root);
}
