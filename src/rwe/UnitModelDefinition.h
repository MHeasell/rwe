#pragma once

#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitPieceDefinition.h>
#include <vector>

namespace rwe
{
    struct UnitModelDefinition
    {
        SimScalar height;
        std::vector<UnitPieceDefinition> pieces;
    };
}
