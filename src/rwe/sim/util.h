#pragma once

#include <rwe/math/Matrix4x.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitMesh.h>
#include <rwe/sim/UnitModelDefinition.h>

namespace rwe
{
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const UnitModelDefinition& modelDefinition, const std::vector<UnitMesh>& pieces);
}
