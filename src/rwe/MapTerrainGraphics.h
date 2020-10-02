#pragma once

#include <rwe/Grid.h>
#include <rwe/SimScalar.h>
#include <rwe/SimVector.h>
#include <rwe/TextureRegion.h>
#include <vector>

namespace rwe
{
    class MapTerrainGraphics
    {
    public:
        static constexpr SimScalar TileWidthInWorldUnits = 32_ss;
        static constexpr SimScalar TileHeightInWorldUnits = 32_ss;

    private:
        std::vector<TextureRegion> tileGraphics;

        Grid<std::size_t> tiles;

    public:
        MapTerrainGraphics(
            std::vector<TextureRegion>&& tileGraphics,
            Grid<std::size_t>&& tiles);

        const TextureRegion& getTileTexture(std::size_t index) const;

        const Grid<std::size_t>& getTiles() const;

        Point worldToTileCoordinate(const SimVector& position) const;

        SimVector tileCoordinateToWorldCorner(int x, int y) const;

        SimScalar getWidthInWorldUnits() const;
        SimScalar getHeightInWorldUnits() const;
    };
}
