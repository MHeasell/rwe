#ifndef RWE_MOVEMENTCLASSCOLLISIONSERVICE_H
#define RWE_MOVEMENTCLASSCOLLISIONSERVICE_H

#include <rwe/GameSimulation.h>
#include <rwe/Grid.h>
#include <rwe/MovementClass.h>
#include <rwe/MovementClassId.h>
#include <rwe/Point.h>
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

        std::optional<MovementClassId> resolveMovementClass(const std::string& name);

        bool isWalkable(MovementClassId movementClass, const Point& position) const;

        const Grid<char>& getGrid(MovementClassId movementClass) const;
    };

    Grid<char> computeWalkableGrid(const GameSimulation& sim, const MovementClass& movementClass);

}

#endif
