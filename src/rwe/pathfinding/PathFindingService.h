#ifndef RWE_PATHFINDINGSERVICE_H
#define RWE_PATHFINDINGSERVICE_H

#include "PathTaskId.h"
#include "PathTaskToken.h"
#include "UnitPath.h"
#include "rwe/GameSimulation.h"
#include "rwe/UnitId.h"
#include <deque>
#include <future>
#include <rwe/math/Vector3f.h>

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

        GameSimulation* const simulation;

        std::deque<std::pair<PathTaskId, PathTask>> taskQueue;

    public:
        PathFindingService(GameSimulation* const simulation);

        PathTaskToken getPath(UnitId unit, const Vector3f& destination);
        void cancelGetPath(PathTaskId task);

        void update();

    private:
        PathTaskId getNextTaskId();

        UnitPath findPath(UnitId unitId, const Vector3f& destination);

        Vector3f getWorldCenter(const DiscreteRect& discreteRect);
    };
}

#endif
