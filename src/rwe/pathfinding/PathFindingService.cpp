#include "PathFindingService.h"
#include "UnitFootprintPathFinder.h"
#include "pathfinding_utils.h"
#include <future>

namespace rwe
{
    static const unsigned int MaxTasksPerTick = 10;

    PathFindingService::PathFindingService(GameSimulation* const simulation) : simulation(simulation)
    {
    }

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

            auto path = findPath(task.second.unit, task.second.destination);
            task.second.result.set_value(std::move(path));

            taskQueue.pop_front();
            tasksDone += 1;
        }
    }

    UnitPath PathFindingService::findPath(UnitId unitId, const Vector3f& destination)
    {
        const auto& unit = simulation->getUnit(unitId);

        auto start = simulation->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto goal = simulation->computeFootprintRegion(destination, unit.footprintX, unit.footprintZ);

        UnitFootprintPathFinder pathFinder(simulation, unitId, unit.footprintX, unit.footprintZ, Point(goal.x, goal.y));

        // TODO: fill in real direction
        auto path = pathFinder.findPath(PathVertex(Point(start.x, start.y), directionFromRadians(unit.rotation)));
        assert(path.size() >= 1);
        auto simplifiedPath = runSimplifyPath(path);

        std::vector<Vector3f> waypoints;
        for (auto it = ++simplifiedPath.cbegin(); it != simplifiedPath.end(); ++it)
        {
            waypoints.push_back(getWorldCenter(DiscreteRect(it->position.x, it->position.y, unit.footprintX, unit.footprintZ)));
        }
        waypoints.back() = destination;

        return UnitPath{std::move(waypoints)};
    }

    Vector3f PathFindingService::getWorldCenter(const DiscreteRect& rect)
    {
        auto corner = simulation->terrain.heightmapIndexToWorldCorner(rect.x, rect.y);

        auto halfWorldWidth = (rect.width * MapTerrain::HeightTileWidthInWorldUnits) / 2.0f;
        auto halfWorldHeight= (rect.height * MapTerrain::HeightTileHeightInWorldUnits) / 2.0f;

        auto center = corner + Vector3f(halfWorldWidth, 0.0f, halfWorldHeight);
        center.y = simulation->terrain.getHeightAt(center.x, center.z);
        return center;
    }
}
