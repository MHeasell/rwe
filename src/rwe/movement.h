#ifndef RWE_MOVEMENT_H
#define RWE_MOVEMENT_H

#include <rwe/Grid.h>
#include <rwe/MapTerrain.h>
#include <rwe/MovementClass.h>
#include <rwe/MovementClassId.h>
#include <rwe/Point.h>
#include <unordered_map>

namespace rwe
{
    bool isGridPointWalkable(const MapTerrain& terrain, const MovementClass& movementClass, unsigned int x, unsigned int y);

    bool isMaxSlopeGreaterThan(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int maxSlope, unsigned int maxWaterSlope);

    bool isWaterDepthWithinBounds(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int minWaterDepth, unsigned int maxWaterDepth);

    unsigned int getWaterDepth(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y);

    unsigned int getSlope(const Grid<unsigned char>& heights, unsigned int x, unsigned int y);

    bool isAreaUnderWater(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
}

#endif
