#include "MovementClassCollisionService.h"
#include <rwe/sim/movement.h>
#include <rwe/util/collection_util.h>

namespace rwe
{
    void MovementClassCollisionService::registerMovementClass(MovementClassId id, Grid<char>&& walkableGrid)
    {
        walkableGrids.insert({id, std::move(walkableGrid)});
    }

    bool MovementClassCollisionService::isWalkable(MovementClassId movementClass, const Point& position) const
    {
        return walkableGrids.at(movementClass).tryGetValue(position).value_or(false);
    }

    const Grid<char>& MovementClassCollisionService::getGrid(MovementClassId movementClass) const
    {
        return walkableGrids.at(movementClass);
    }
    Grid<char> computeWalkableGrid(const MapTerrain& terrain, const MovementClassDefinition& movementClass)
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
