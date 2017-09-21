#ifndef RWE_MAPTERRAIN_H
#define RWE_MAPTERRAIN_H

#include <vector>
#include <rwe/TextureRegion.h>
#include <rwe/Point.h>
#include <rwe/Grid.h>
#include <rwe/camera/CabinetCamera.h>

namespace rwe
{
    class GraphicsContext;

    class MapTerrain
    {
    public:
        static constexpr float TileWidthInWorldUnits = 32.0f;
        static constexpr float TileHeightInWorldUnits = 32.0f;

    private:
        std::vector<TextureRegion> tileGraphics;

        Grid<std::size_t> tiles;

        Grid<unsigned char> heights;

    public:
        MapTerrain(
            std::vector<TextureRegion>&& tileGraphics,
            Grid<size_t>&& tiles,
            Grid<unsigned char>&& heights);

        void render(GraphicsContext& graphics, const CabinetCamera& cabinetCamera) const;

        Point worldToTileCoordinate(const Vector3f& position) const;

        Vector3f tileCoordinateToWorldCorner(int x, int y) const;

        const TextureRegion& getTileTexture(std::size_t index) const;

        const Grid<std::size_t>& getTiles() const;
    };
}

#endif
