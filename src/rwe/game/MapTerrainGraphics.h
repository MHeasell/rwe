#pragma once

#include <rwe/grid/Grid.h>
#include <rwe/render/TextureArrayRegion.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <vector>

namespace rwe
{
    class MapTerrainGraphics
    {
    public:
        static constexpr SimScalar TileWidthInWorldUnits = 32_ss;
        static constexpr SimScalar TileHeightInWorldUnits = 32_ss;

    private:
        std::vector<TextureArrayRegion> tileGraphics;

        Grid<std::size_t> tiles;

    public:
        MapTerrainGraphics(
            std::vector<TextureArrayRegion>&& tileGraphics,
            Grid<std::size_t>&& tiles);

        const TextureArrayRegion& getTileTexture(std::size_t index) const;

        const Grid<std::size_t>& getTiles() const;

        Point worldToTileCoordinate(const SimVector& position) const;

        SimVector tileCoordinateToWorldCorner(int x, int y) const;

        SimScalar getWidthInWorldUnits() const;
        SimScalar getHeightInWorldUnits() const;
    };
}
