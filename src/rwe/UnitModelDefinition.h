#pragma once

#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitPieceDefinition.h>

namespace rwe
{
    struct UnitModelDefinition
    {
        SimScalar height;
        UnitPieceDefinition rootPiece;
    };
}
