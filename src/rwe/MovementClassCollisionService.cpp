#include "MovementClassCollisionService.h"
#include <rwe/collection_util.h>
#include <rwe/sim/movement.h>

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

    std::optional<MovementClassId> MovementClassCollisionService::resolveMovementClass(const std::string& name) const
    {
        return tryFindValue(movementClassNameMap, name);
    }

    bool MovementClassCollisionService::isWalkable(MovementClassId movementClass, const Point& position) const
    {
        return walkableGrids.at(movementClass).tryGetValue(position).value_or(false);
    }

    const Grid<char>& MovementClassCollisionService::getGrid(MovementClassId movementClass) const
    {
        return walkableGrids.at(movementClass);
    }

    Grid<char> computeWalkableGrid(const MapTerrain& terrain, const MovementClass& movementClass)
    {
        const auto footprintX = movementClass.footprintX;
        const auto footprintY = movementClass.footprintZ;

        const auto width = terrain.getHeightMap().getWidth() - footprintX;
        const auto height = terrain.getHeightMap().getHeight() - footprintY;

        return Grid<char>::from(
            width,
            height,
            [&](const auto& c) { return isGridPointWalkable(terrain, movementClass, c.x, c.y); });
    }
}
