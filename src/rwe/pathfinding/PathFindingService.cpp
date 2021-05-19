#include "PathFindingService.h"
#include <future>
#include <rwe/pathfinding/UnitPathFinder.h>
#include <rwe/pathfinding/UnitPerimeterPathFinder.h>
#include <rwe/pathfinding/pathfinding_utils.h>

namespace rwe
{
    static const unsigned int MaxTasksPerTick = 10;

    PathFindingService::PathFindingService(GameSimulation* simulation, MovementClassCollisionService* collisionService)
        : simulation(simulation), collisionService(collisionService)
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

            if (auto movingState = std::get_if<MovingState>(&unit.behaviourState); movingState != nullptr)
            {
                UnitPath path = std::visit(FindPathVisitor(this, request.unitId), movingState->destination);
                movingState->path = PathFollowingInfo(std::move(path), simulation->gameTime);
                movingState->pathRequested = false;
            }

            requests.pop_front();
            tasksDone += 1;
        }
    }

    UnitPath PathFindingService::findPath(UnitId unitId, const DiscreteRect& destination)
    {
        const auto& unit = simulation->getUnit(unitId);

        auto start = simulation->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        // expand the goal rect to take into account our own collision rect
        auto goal = destination.expandTopLeft(unit.footprintX, unit.footprintZ);

        UnitPerimeterPathFinder pathFinder(simulation, collisionService, unitId, unit.movementClass, unit.footprintX, unit.footprintZ, goal);

        auto path = pathFinder.findPath(Point(start.x, start.y));
        lastPathDebugInfo = AStarPathInfo<Point, PathCost>{path.type, path.path, std::move(path.closedVertices)};

        assert(path.path.size() >= 1);

        if (path.path.size() == 1)
        {
            // The path is trivial, we are already at the goal.
            return UnitPath{std::vector<SimVector>{unit.position}};
        }

        auto simplifiedPath = runSimplifyPath(path.path);

        std::vector<SimVector> waypoints;
        for (auto it = ++simplifiedPath.cbegin(); it != simplifiedPath.cend(); ++it)
        {
            waypoints.push_back(getWorldCenter(DiscreteRect(it->x, it->y, unit.footprintX, unit.footprintZ)));
        }

        return UnitPath{std::move(waypoints)};
    }

    UnitPath PathFindingService::findPath(UnitId unitId, const SimVector& destination)
    {
        const auto& unit = simulation->getUnit(unitId);

        auto start = simulation->computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto goal = simulation->computeFootprintRegion(destination, unit.footprintX, unit.footprintZ);

        UnitPathFinder pathFinder(simulation, collisionService, unitId, unit.movementClass, unit.footprintX, unit.footprintZ, Point(goal.x, goal.y));

        auto path = pathFinder.findPath(Point(start.x, start.y));
        lastPathDebugInfo = AStarPathInfo<Point, PathCost>{path.type, path.path, std::move(path.closedVertices)};

        if (path.type == AStarPathType::Partial)
        {
            path.path.emplace_back(goal.x, goal.y);
        }

        assert(path.path.size() >= 1);

        if (path.path.size() == 1)
        {
            // The path is trivial, we are already at the goal.
            return UnitPath{std::vector<SimVector>{destination}};
        }

        auto simplifiedPath = runSimplifyPath(path.path);

        std::vector<SimVector> waypoints;
        for (auto it = ++simplifiedPath.cbegin(); it != simplifiedPath.cend(); ++it)
        {
            waypoints.push_back(getWorldCenter(DiscreteRect(it->x, it->y, unit.footprintX, unit.footprintZ)));
        }
        waypoints.back() = destination;

        return UnitPath{std::move(waypoints)};
    }

    SimVector PathFindingService::getWorldCenter(const DiscreteRect& rect)
    {
        auto corner = simulation->terrain.heightmapIndexToWorldCorner(rect.x, rect.y);

        auto halfWorldWidth = (simScalarFromInt(rect.width) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss;
        auto halfWorldHeight = (simScalarFromInt(rect.height) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss;

        auto center = corner + SimVector(halfWorldWidth, 0_ss, halfWorldHeight);
        center.y = simulation->terrain.getHeightAt(center.x, center.z);
        return center;
    }
}
