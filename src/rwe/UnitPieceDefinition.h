#pragma once

#include <rwe/SimVector.h>
#include <string>

namespace rwe
{
    struct UnitPieceDefinition
    {
        std::string name;
        SimVector origin;
        std::vector<UnitPieceDefinition> children;
    };
}
