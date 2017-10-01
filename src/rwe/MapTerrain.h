#ifndef RWE_MAPTERRAIN_H
#define RWE_MAPTERRAIN_H

#include <rwe/Grid.h>
#include <rwe/MapFeature.h>
#include <rwe/Point.h>
#include <rwe/TextureRegion.h>
#include <rwe/camera/CabinetCamera.h>
#include <vector>

namespace rwe
{
    class GraphicsContext;

    class MapTerrain
    {
    public:
        static constexpr float TileWidthInWorldUnits = 32.0f;
        static constexpr float TileHeightInWorldUnits = 32.0f;

        static constexpr float HeightTileWidthInWorldUnits = 16.0f;
        static constexpr float HeightTileHeightInWorldUnits = 16.0f;

    private:
        std::vector<TextureRegion> tileGraphics;

        Grid<std::size_t> tiles;

        Grid<unsigned char> heights;

        std::vector<MapFeature> features;

    public:
        MapTerrain(
            std::vector<TextureRegion>&& tileGraphics,
            Grid<size_t>&& tiles,
            Grid<unsigned char>&& heights);

        void render(GraphicsContext& graphics, const CabinetCamera& cabinetCamera) const;

        void renderFeatures(GraphicsContext& graphics, const CabinetCamera& cabinetCamera) const;

        Point worldToTileCoordinate(const Vector3f& position) const;

        Vector3f tileCoordinateToWorldCorner(int x, int y) const;

        Vector3f heightmapIndexToWorldCorner(std::size_t x, std::size_t y) const;

        const TextureRegion& getTileTexture(std::size_t index) const;

        const Grid<std::size_t>& getTiles() const;

        const Grid<unsigned char>& getHeightMap() const;

        float leftInWorldUnits() const;
        float rightCutoffInWorldUnits() const;
        float topInWorldUnits() const;
        float bottomCutoffInWorldUnits() const;

        const std::vector<MapFeature>& getFeatures() const;
        std::vector<MapFeature>& getFeatures();
    };
}

#endif
