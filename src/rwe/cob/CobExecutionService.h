#pragma once

#include <rwe/GameSimulation.h>

namespace rwe
{
    class GameScene;

    class CobExecutionService
    {
    public:
        void run(GameScene& scene, GameSimulation& simulation, UnitId unitId);
    };
}
