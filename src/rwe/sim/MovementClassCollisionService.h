#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/grid/Point.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClassDefinition.h>
#include <rwe/sim/MovementClassId.h>
#include <unordered_map>

namespace rwe
{
    class MovementClassCollisionService
    {
    private:
        std::unordered_map<MovementClassId, Grid<char>> walkableGrids;

    public:
        void registerMovementClass(MovementClassId id, Grid<char>&& walkableGrid);

        bool isWalkable(MovementClassId movementClass, const Point& position) const;

        const Grid<char>& getGrid(MovementClassId movementClass) const;
    };

    Grid<char> computeWalkableGrid(const MapTerrain& terrain, const MovementClassDefinition& movementClass);
}
