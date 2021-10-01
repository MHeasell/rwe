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
        std::unordered_map<std::string, int> pieceIndicesByName;
    };

    UnitModelDefinition createUnitModelDefinition(SimScalar height, std::vector<UnitPieceDefinition>&& pieces);
}
