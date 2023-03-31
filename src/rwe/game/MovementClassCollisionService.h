#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/grid/Point.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/MovementClassId.h>
#include <unordered_map>

namespace rwe
{
    class MovementClassCollisionService
    {
    private:
        unsigned int nextId{0};

        std::unordered_map<std::string, MovementClassId> movementClassNameMap;
        std::unordered_map<MovementClassId, Grid<char>> walkableGrids;

    public:
        MovementClassId registerMovementClass(const std::string& className, Grid<char>&& walkableGrid);

        std::optional<MovementClassId> resolveMovementClass(const std::string& name) const;

        bool isWalkable(MovementClassId movementClass, const Point& position) const;

        const Grid<char>& getGrid(MovementClassId movementClass) const;
    };

    Grid<char> computeWalkableGrid(const MapTerrain& terrain, const MovementClass& movementClass);
}
