#pragma once

#include <rwe/sim/GameSimulation.h>

namespace rwe
{
    class GameScene;

    class CobExecutionService
    {
    public:
        void run(GameSimulation& simulation, UnitId unitId);
    };
}
