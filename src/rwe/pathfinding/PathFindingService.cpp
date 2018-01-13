#include "PathFindingService.h"
#include "UnitFootprintPathFinder.h"
#include "pathfinding_utils.h"
#include <future>

namespace rwe
{
    static const unsigned int MaxTasksPerTick = 10;

    PathFindingService::PathFindingService(GameSimulation* simulation) : simulation(simulation)
    {
    }

    void PathFindingService::update()
    {
        auto& requests = simulation->pathRequests;
        unsigned int tasksDone = 0;
        while (!requests.empty() && tasksDone < MaxTasksPerTick)
        {
            auto& request = requests.front();

            auto& unit = simulation->getUnit(request.unitId);

            auto pathRequest = boost::get<PathStatusRequested>(&*unit.pathStatus);
            if (pathRequest != nullptr)
            {
                auto path = findPath(request.unitId, pathRequest->destination);
                unit.pathStatus = PathStatusFollowing(std::move(path));
            }

            requests.pop_front();
            tasksDone += 1;
        }
    }

    UnitPath PathFindingService::findPath(UnitId unitId, const Vector3f& destination)
    {
        const auto& unit = simulation->getUnit(unitId);

        auto start = simulation->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto goal = simulation->computeFootprintRegion(destination, unit.footprintX, unit.footprintZ);

        UnitFootprintPathFinder pathFinder(simulation, unitId, unit.footprintX, unit.footprintZ, Point(goal.x, goal.y));

        auto path = pathFinder.findPath(Point(start.x, start.y));
        if (path.type == AStarPathType::Partial)
        {
            path.path.emplace_back(goal.x, goal.y);
        }

        assert(path.path.size() >= 1);
        auto simplifiedPath = runSimplifyPath(path.path);

        std::vector<Vector3f> waypoints;
        for (auto it = ++simplifiedPath.cbegin(); it != simplifiedPath.end(); ++it)
        {
            waypoints.push_back(getWorldCenter(DiscreteRect(it->x, it->y, unit.footprintX, unit.footprintZ)));
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
