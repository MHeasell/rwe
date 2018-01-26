#ifndef RWE_MOVEMENTCLASSCOLLISIONSERVICE_H
#define RWE_MOVEMENTCLASSCOLLISIONSERVICE_H

#include "GameSimulation.h"
#include "MovementClass.h"
#include "MovementClassId.h"
#include "Point.h"
#include <rwe/Grid.h>
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

        boost::optional<MovementClassId> resolveMovementClass(const std::string& name);

        bool isWalkable(MovementClassId movementClass, const Point& position) const;
    };

    Grid<char> computeWalkableGrid(const GameSimulation& sim, const MovementClass& movementClass);

    bool isGridPointWalkable(const MapTerrain& terrain, const MovementClass& movementClass, unsigned int x, unsigned int y);

    bool isMaxSlopeGreaterThan(const Grid<unsigned char>& heights, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int maxSlope);

    bool isWaterDepthWithinBounds(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int minWaterDepth, unsigned int maxWaterDepth);

    unsigned int getWaterDepth(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y);
}

#endif
