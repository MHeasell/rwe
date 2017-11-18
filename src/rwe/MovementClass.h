#ifndef RWE_MOVEMENTCLASS_H
#define RWE_MOVEMENTCLASS_H

#include <string>
#include <rwe/tdf/TdfBlock.h>

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
    };

    MovementClass parseMovementClass(const TdfBlock& block);

    std::vector<std::pair<std::string, MovementClass>> parseMovementTdf(const TdfBlock& root);
}

#endif
