#pragma once

#include <rwe/math/Matrix4x.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitMesh.h>
#include <rwe/sim/UnitPieceDefinition.h>
#include <vector>

namespace rwe
{
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const std::vector<UnitPieceDefinition>& pieceDefinitions, const std::vector<UnitMesh>& pieces);
}
