#pragma once

#include <rwe/sim/GameSimulation.h>

namespace rwe
{
    class CobExecutionService
    {
    public:
        void run(GameSimulation& simulation, UnitId unitId);
    };
}
