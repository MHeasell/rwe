#include "MapTerrainGraphics.h"

namespace rwe
{
    MapTerrainGraphics::MapTerrainGraphics(std::vector<TextureArrayRegion>&& tileGraphics, Grid<std::size_t>&& tiles)
        : tileGraphics(std::move(tileGraphics)), tiles(std::move(tiles))
    {
    }

    const TextureArrayRegion& MapTerrainGraphics::getTileTexture(std::size_t index) const
    {
        assert(index < tileGraphics.size());
        return tileGraphics[index];
    }

    const Grid<std::size_t>& MapTerrainGraphics::getTiles() const
    {
        return tiles;
    }

    Point MapTerrainGraphics::worldToTileCoordinate(const SimVector& position) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto newX = (position.x + (widthInWorldUnits / 2_ss)) / TileWidthInWorldUnits;
        auto newY = (position.z + (heightInWorldUnits / 2_ss)) / TileHeightInWorldUnits;

        return Point(static_cast<int>(newX.value), static_cast<int>(newY.value));
    }

    SimVector MapTerrainGraphics::tileCoordinateToWorldCorner(int x, int y) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto worldX = (SimScalar(x) * TileWidthInWorldUnits) - (widthInWorldUnits / 2_ss);
        auto worldY = (SimScalar(y) * TileHeightInWorldUnits) - (heightInWorldUnits / 2_ss);

        return SimVector(worldX, 0_ss, worldY);
    }

    SimScalar MapTerrainGraphics::getWidthInWorldUnits() const
    {
        return SimScalar(tiles.getWidth()) * TileWidthInWorldUnits;
    }

    SimScalar MapTerrainGraphics::getHeightInWorldUnits() const
    {
        return SimScalar(tiles.getHeight()) * TileHeightInWorldUnits;
    }
}
