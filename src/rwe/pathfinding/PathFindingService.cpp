#include "PathFindingService.h"
#include <rwe/pathfinding/UnitPathFinder.h>
#include <rwe/pathfinding/UnitPerimeterPathFinder.h>
#include <rwe/pathfinding/pathfinding_utils.h>

namespace rwe
{
    static const unsigned int MaxTasksPerTick = 10;

    void PathFindingService::update(GameSimulation& simulation)
    {
        auto& requests = simulation.pathRequests;
        unsigned int tasksDone = 0;
        while (!requests.empty() && tasksDone < MaxTasksPerTick)
        {
            auto& request = requests.front();

            auto unit = simulation.tryGetUnitState(request.unitId);
            if (!unit)
            {
                // Unit that made the request no longer exists.
                // Possibly the unit died. Just skip it.
                requests.pop_front();
                continue;
            }

            if (auto movingState = std::get_if<NavigationStateMoving>(&unit->get().navigationState.state); movingState != nullptr)
            {
                auto path = match(
                    movingState->pathDestination,
                    [&](const SimVector& pos) {
                        return findPath(simulation, request.unitId, pos);
                    },
                    [&](const DiscreteRect& pos) {
                        return findPath(simulation, request.unitId, pos);
                    });
                movingState->path = PathFollowingInfo(std::move(path), simulation.gameTime);
                movingState->pathRequested = false;
            }

            requests.pop_front();
            tasksDone += 1;
        }
    }

    UnitPath PathFindingService::findPath(const GameSimulation& simulation, UnitId unitId, const DiscreteRect& destination)
    {
        const auto& unit = simulation.getUnitState(unitId);
        const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);

        auto start = simulation.computeFootprintRegion(unit.position, unitDefinition.movementCollisionInfo);
        // expand the goal rect to take into account our own collision rect
        auto goal = destination.expandTopLeft(start.width, start.height);

        auto movementClassId = match(
            unitDefinition.movementCollisionInfo, [&](const UnitDefinition::NamedMovementClass& mc) { return std::make_optional(mc.movementClassId); }, [&](const auto&) { return std::optional<MovementClassId>(); });

        UnitPerimeterPathFinder pathFinder(&simulation, &simulation.movementClassCollisionService, unitId, movementClassId, start.width, start.height, goal);

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
            waypoints.push_back(getWorldCenter(simulation, DiscreteRect(it->x, it->y, start.width, start.height)));
        }

        return UnitPath{std::move(waypoints)};
    }

    UnitPath PathFindingService::findPath(const GameSimulation& simulation, UnitId unitId, const SimVector& destination)
    {
        const auto& unit = simulation.getUnitState(unitId);
        const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);

        auto start = simulation.computeFootprintRegion(unit.position, unitDefinition.movementCollisionInfo);
        auto goal = simulation.computeFootprintRegion(destination, unitDefinition.movementCollisionInfo);

        auto movementClassId = match(
            unitDefinition.movementCollisionInfo, [&](const UnitDefinition::NamedMovementClass& mc) { return std::make_optional(mc.movementClassId); }, [&](const auto&) { return std::optional<MovementClassId>(); });

        UnitPathFinder pathFinder(&simulation, &simulation.movementClassCollisionService, unitId, movementClassId, start.width, start.height, Point(goal.x, goal.y));

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
            waypoints.push_back(getWorldCenter(simulation, DiscreteRect(it->x, it->y, start.width, start.height)));
        }
        waypoints.back() = destination;

        return UnitPath{std::move(waypoints)};
    }

    SimVector PathFindingService::getWorldCenter(const GameSimulation& simulation, const DiscreteRect& rect)
    {
        auto corner = simulation.terrain.heightmapIndexToWorldCorner(rect.x, rect.y);

        auto halfWorldWidth = (SimScalar(rect.width) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss;
        auto halfWorldHeight = (SimScalar(rect.height) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss;

        auto center = corner + SimVector(halfWorldWidth, 0_ss, halfWorldHeight);
        center.y = simulation.terrain.getHeightAt(center.x, center.z);
        return center;
    }
}
