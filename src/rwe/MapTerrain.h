#ifndef RWE_MAPTERRAIN_H
#define RWE_MAPTERRAIN_H

#include <rwe/Grid.h>
#include <rwe/Point.h>
#include <rwe/TextureRegion.h>
#include <rwe/camera/CabinetCamera.h>
#include <rwe/geometry/Line3f.h>
#include <vector>

namespace rwe
{
    class MapTerrain
    {
    public:
        static constexpr float TileWidthInWorldUnits = 32.0f;
        static constexpr float TileHeightInWorldUnits = 32.0f;

        static constexpr float HeightTileWidthInWorldUnits = 16.0f;
        static constexpr float HeightTileHeightInWorldUnits = 16.0f;

        static constexpr float MaxHeight = 255.0f;
        static constexpr float MinHeight = 0.0f;

    private:
        std::vector<TextureRegion> tileGraphics;

        Grid<std::size_t> tiles;

        Grid<unsigned char> heights;

        float seaLevel;

    public:
        MapTerrain(
            std::vector<TextureRegion>&& tileGraphics,
            Grid<size_t>&& tiles,
            Grid<unsigned char>&& heights,
            float seaLevel);

        Point worldToTileCoordinate(const Vector3f& position) const;

        Vector3f tileCoordinateToWorldCorner(int x, int y) const;

        Point worldToHeightmapCoordinate(const Vector3f& position) const;

        Point worldToHeightmapCoordinateNearest(const Vector3f& position) const;

        Vector3f heightmapIndexToWorldCorner(int x, int y) const;

        Vector3f heightmapIndexToWorldCorner(Point p) const;

        Vector3f heightmapIndexToWorldCenter(int x, int y) const;

        Vector3f heightmapIndexToWorldCenter(std::size_t x, std::size_t y) const;

        Vector3f heightmapIndexToWorldCenter(Point p) const;

        Vector3f worldToHeightmapSpace(const Vector3f& v) const;

        Vector3f heightmapToWorldSpace(const Vector3f& v) const;

        Vector3f topLeftCoordinateToWorld(const Vector3f& position) const;

        const TextureRegion& getTileTexture(std::size_t index) const;

        const Grid<std::size_t>& getTiles() const;

        const Grid<unsigned char>& getHeightMap() const;

        float leftInWorldUnits() const;
        float rightCutoffInWorldUnits() const;
        float topInWorldUnits() const;
        float bottomCutoffInWorldUnits() const;

        float getWidthInWorldUnits() const;
        float getHeightInWorldUnits() const;

        /**
         * Gets the height of the terrain at the given world coordinates.
         * If the input is outside the heightmap grid, returns 0.
         */
        float getHeightAt(float x, float z) const;

        std::optional<Vector3f> intersectLine(const Line3f& line) const;

        std::optional<Vector3f> intersectWithHeightmapCell(const Line3f& line, int x, int y) const;

        float getSeaLevel() const;

    private:
        bool isInHeightMapBounds(int x, int y) const;
    };
}

#endif
