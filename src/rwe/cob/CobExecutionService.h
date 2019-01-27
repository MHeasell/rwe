#ifndef RWE_COBEXECUTIONSERVICE_H
#define RWE_COBEXECUTIONSERVICE_H

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

#endif
