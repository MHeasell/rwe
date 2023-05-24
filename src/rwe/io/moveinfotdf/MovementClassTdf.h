#pragma once

#include <string>

namespace rwe
{
    struct MovementClassTdf
    {
        std::string name;
        unsigned int footprintX;
        unsigned int footprintZ;
        unsigned int minWaterDepth;
        unsigned int maxWaterDepth;
        unsigned int maxSlope;
        unsigned int maxWaterSlope;
    };
}
