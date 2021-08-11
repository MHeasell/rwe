#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/grid/Point.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClass.h>
#include <rwe/sim/MovementClassId.h>
#include <unordered_map>

namespace rwe
{
    bool isGridPointWalkable(const MapTerrain& terrain, const MovementClass& movementClass, int x, int y);

    bool isMaxSlopeGreaterThan(const Grid<unsigned char>& heights, int waterLevel, int x, int y, int width, int height, int maxSlope, int maxWaterSlope);

    bool isWaterDepthWithinBounds(const Grid<unsigned char>& heights, int waterLevel, int x, int y, int width, int height, int minWaterDepth, int maxWaterDepth);

    int getWaterDepth(const Grid<unsigned char>& heights, int waterLevel, int x, int y);

    int getSlope(const Grid<unsigned char>& heights, int x, int y);

    bool isAreaUnderWater(const Grid<unsigned char>& heights, int waterLevel, int x, int y, int width, int height);
}
