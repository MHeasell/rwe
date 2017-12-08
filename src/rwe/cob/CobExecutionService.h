#ifndef RWE_COBEXECUTIONSERVICE_H
#define RWE_COBEXECUTIONSERVICE_H

#include <rwe/GameSimulation.h>

namespace rwe
{
    class CobExecutionService
    {
    public:
        void run(GameSimulation& simulation, UnitId unitId);
    };
}

#endif
