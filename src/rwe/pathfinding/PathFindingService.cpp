#include "PathFindingService.h"
#include <future>

namespace rwe
{
    static const unsigned int MaxTasksPerTick = 10;

    PathTaskToken PathFindingService::getPath(UnitId unit, const Vector3f& destination)
    {
        auto taskId = getNextTaskId();
        auto& task = taskQueue.emplace_back(taskId, PathTask{unit, destination, std::promise<UnitPath>()});
        return {taskId, task.second.result.get_future()};
    }

    PathTaskId PathFindingService::getNextTaskId()
    {
        return PathTaskId(nextId++);
    }

    void PathFindingService::cancelGetPath(PathTaskId taskId)
    {
        auto it = std::find_if(taskQueue.begin(), taskQueue.end(), [taskId](const auto& pair) { return pair.first == taskId; });
        if (it != taskQueue.end())
        {
            taskQueue.erase(it);
        }
    }

    void PathFindingService::update()
    {
        unsigned int tasksDone = 0;
        while (!taskQueue.empty() && tasksDone < MaxTasksPerTick)
        {
            auto& task = taskQueue.front();

            // TODO: write a real pathfinding algo
            UnitPath path;
            path.waypoints.push_back(task.second.destination);
            task.second.result.set_value(std::move(path));

            taskQueue.pop_front();
            tasksDone += 1;
        }
    }
}
