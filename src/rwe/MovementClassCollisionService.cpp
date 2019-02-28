#include "MovementClassCollisionService.h"
#include <rwe/movement.h>

namespace rwe
{
    MovementClassId
    MovementClassCollisionService::registerMovementClass(const std::string& className, Grid<char>&& walkableGrid)
    {
        MovementClassId id(nextId++);
        walkableGrids.insert({id, std::move(walkableGrid)});
        movementClassNameMap.insert({className, id});
        return id;
    }

    std::optional<MovementClassId> MovementClassCollisionService::resolveMovementClass(const std::string& name)
    {
        auto it = movementClassNameMap.find(name);
        if (it == movementClassNameMap.end())
        {
            return std::nullopt;
        }

        return it->second;
    }

    bool MovementClassCollisionService::isWalkable(MovementClassId movementClass, const Point& position) const
    {
        if (position.x < 0 || position.y < 0)
        {
            return false;
        }

        auto x = static_cast<unsigned int>(position.x);
        auto y = static_cast<unsigned int>(position.y);

        const auto& map = walkableGrids.find(movementClass);
        if (map == walkableGrids.end())
        {
            throw std::logic_error("Grid for movement class not found");
        }

        const auto& grid = map->second;
        if (x >= grid.getWidth() || y >= grid.getHeight())
        {
            return false;
        }

        return grid.get(x, y);
    }

    const Grid<char>& MovementClassCollisionService::getGrid(MovementClassId movementClass) const
    {
        auto it = walkableGrids.find(movementClass);
        if (it == walkableGrids.end())
        {
            throw std::runtime_error("Failed to find movement class grid");
        }

        return it->second;
    }

    Grid<char> computeWalkableGrid(const GameSimulation& sim, const MovementClass& movementClass)
    {
        const auto& terrain = sim.terrain;

        const auto width = terrain.getHeightMap().getWidth();
        const auto height = terrain.getHeightMap().getHeight();

        Grid<char> walkableGrid(width, height, false);

        const auto footprintX = movementClass.footprintX;
        const auto footprintY = movementClass.footprintZ;

        for (unsigned int y = 0; y < height - footprintY - 1; ++y)
        {
            for (unsigned int x = 0; x < width - footprintX - 1; ++x)
            {
                walkableGrid.set(x, y, isGridPointWalkable(terrain, movementClass, x, y));
            }
        }

        return walkableGrid;
    }
}
