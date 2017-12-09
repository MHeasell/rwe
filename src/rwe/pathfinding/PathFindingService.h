#ifndef RWE_PATHFINDINGSERVICE_H
#define RWE_PATHFINDINGSERVICE_H

#include "rwe/UnitId.h"
#include "rwe/GameSimulation.h"
#include "PathTaskId.h"
#include "UnitPath.h"
#include "PathTaskToken.h"
#include <future>
#include <rwe/math/Vector3f.h>
#include <deque>

namespace rwe
{
    class PathFindingService
    {
    public:

    private:
        struct PathTask
        {
            UnitId unit;
            Vector3f destination;
            std::promise<UnitPath> result;
        };

    private:
        unsigned int nextId;

        GameSimulation* simulation;

        std::deque<std::pair<PathTaskId, PathTask>> taskQueue;

    public:
        PathTaskToken getPath(UnitId unit, const Vector3f& destination);
        void cancelGetPath(PathTaskId task);

        void update();

    private:
        PathTaskId getNextTaskId();
    };
}

#endif
