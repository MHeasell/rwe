#pragma once

#include <rwe/SimScalar.h>
#include <rwe/UnitPieceDefinition.h>

namespace rwe
{
    struct UnitModelDefinition
    {
        SimScalar height;
        UnitPieceDefinition rootPiece;
    };
}
