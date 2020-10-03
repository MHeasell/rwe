#pragma once

#include <rwe/sim/GameSimulation.h>

namespace rwe
{
    class GameScene;

    class CobExecutionService
    {
    public:
        void run(GameScene& scene, GameSimulation& simulation, UnitId unitId);
    };
}
