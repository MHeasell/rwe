#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/grid/Point.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/MovementClassDefinition.h>
#include <rwe/sim/MovementClassId.h>
#include <unordered_map>

namespace rwe
{
    bool isGridPointWalkable(const MapTerrain& terrain, const MovementClassDefinition& movementClass, unsigned int x, unsigned int y);

    bool isMaxSlopeGreaterThan(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int maxSlope, unsigned int maxWaterSlope);

    bool isWaterDepthWithinBounds(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int minWaterDepth, unsigned int maxWaterDepth);

    unsigned int getWaterDepth(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y);

    unsigned int getSlope(const Grid<unsigned char>& heights, unsigned int x, unsigned int y);

    bool isAreaUnderWater(const Grid<unsigned char>& heights, unsigned int waterLevel, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
}
