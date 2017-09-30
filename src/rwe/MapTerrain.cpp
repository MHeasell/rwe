#include "MapTerrain.h"
#include <rwe/GraphicsContext.h>
#include <rwe/camera/CabinetCamera.h>

namespace rwe
{
    MapTerrain::MapTerrain(
        std::vector<TextureRegion>&& tileGraphics,
        Grid<size_t>&& tiles,
        Grid<unsigned char>&& heights)
        : tileGraphics(std::move(tileGraphics)), tiles(std::move(tiles)), heights(std::move(heights))
    {
    }

    void MapTerrain::render(GraphicsContext& graphics, const CabinetCamera& cabinetCamera) const
    {
        Vector3f cameraExtents(cabinetCamera.getWidth() / 2.0f, 0.0f, cabinetCamera.getHeight() / 2.0f);
        auto topLeft = worldToTileCoordinate(cabinetCamera.getPosition() - cameraExtents);
        auto bottomRight = worldToTileCoordinate(cabinetCamera.getPosition() + cameraExtents);
        auto x1 = static_cast<unsigned int>(std::clamp<int>(topLeft.x, 0, tiles.getWidth() - 1));
        auto y1 = static_cast<unsigned int>(std::clamp<int>(topLeft.y, 0, tiles.getHeight() - 1));
        auto x2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.x, 0, tiles.getWidth() - 1));
        auto y2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.y, 0, tiles.getHeight() - 1));

        graphics.drawMapTerrain(*this, x1, y1, (x2 + 1) - x1, (y2 + 1) - y1);
    }

    void MapTerrain::renderFeatures(GraphicsContext& graphics, const CabinetCamera& cabinetCamera) const
    {
        for (const auto& f : features)
        {
            graphics.drawFeature(f);
        }
    }

    const TextureRegion& MapTerrain::getTileTexture(std::size_t index) const
    {
        assert(index < tileGraphics.size());
        return tileGraphics[index];
    }

    const Grid<std::size_t>& MapTerrain::getTiles() const
    {
        return tiles;
    }

    Point MapTerrain::worldToTileCoordinate(const Vector3f& position) const
    {
        auto widthInWorldUnits = tiles.getWidth() * TileWidthInWorldUnits;
        auto heightInWorldUnits = tiles.getHeight() * TileHeightInWorldUnits;

        auto newX = (position.x + (widthInWorldUnits / 2.0f)) / TileWidthInWorldUnits;
        auto newY = (position.z + (heightInWorldUnits / 2.0f)) / TileHeightInWorldUnits;

        return Point(static_cast<int>(newX), static_cast<int>(newY));
    }

    Vector3f MapTerrain::tileCoordinateToWorldCorner(int x, int y) const
    {
        auto widthInWorldUnits = tiles.getWidth() * TileWidthInWorldUnits;
        auto heightInWorldUnits = tiles.getHeight() * TileHeightInWorldUnits;

        auto worldX = (x * TileWidthInWorldUnits) - (widthInWorldUnits / 2.0f);
        auto worldY = (y * TileHeightInWorldUnits) - (heightInWorldUnits / 2.0f);

        return Vector3f(worldX, 0.0f, worldY);
    }

    Vector3f MapTerrain::heightmapIndexToWorldCorner(std::size_t x, std::size_t y) const
    {
        auto widthInWorldUnits = heights.getWidth() * HeightTileWidthInWorldUnits;
        auto heightInWorldUnits = heights.getHeight() * HeightTileHeightInWorldUnits;
        auto worldX = (x * HeightTileWidthInWorldUnits) - (widthInWorldUnits / 2.0f);
        auto worldZ = (y * HeightTileHeightInWorldUnits) - (heightInWorldUnits / 2.0f);
        auto worldY = static_cast<float>(heights.get(x, y));

        return Vector3f(worldX, worldY, worldZ);
    }

    float MapTerrain::leftInWorldUnits() const
    {
        return -((static_cast<float>(tiles.getWidth()) / 2.0f) * TileWidthInWorldUnits);
    }

    float MapTerrain::rightCutoffInWorldUnits() const
    {
        auto right = (static_cast<float>(tiles.getWidth()) / 2.0f) * TileWidthInWorldUnits;
        return right - TileWidthInWorldUnits;
    }

    float MapTerrain::topInWorldUnits() const
    {
        return -((static_cast<float>(tiles.getHeight()) / 2.0f) * TileHeightInWorldUnits);
    }

    float MapTerrain::bottomCutoffInWorldUnits() const
    {
        auto bottom = (static_cast<float>(tiles.getHeight()) / 2.0f) * TileHeightInWorldUnits;
        return bottom - (TileHeightInWorldUnits * 4);
    }

    const std::vector<MapFeature>& MapTerrain::getFeatures() const
    {
        return features;
    }

    std::vector<MapFeature>& MapTerrain::getFeatures()
    {
        return features;
    }
}
