#pragma once

#include <rwe/cob/CobAngle.h>
#include <rwe/sim/GameSimulation.h>
#include <rwe/sim/UnitId.h>

namespace rwe
{
    CobAngle toCobAngle(SimAngle angle);

    CobSpeed toCobSpeed(SimScalar speed);

    void runUnitCobScripts(GameSimulation& simulation, UnitId unitId);
}
