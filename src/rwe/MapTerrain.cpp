#include "MapTerrain.h"
#include <rwe/GraphicsContext.h>
#include <rwe/camera/CabinetCamera.h>
#include <rwe/geometry/Plane3f.h>
#include <rwe/geometry/Triangle3f.h>

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
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto newX = (position.x + (widthInWorldUnits / 2.0f)) / TileWidthInWorldUnits;
        auto newY = (position.z + (heightInWorldUnits / 2.0f)) / TileHeightInWorldUnits;

        return Point(static_cast<int>(newX), static_cast<int>(newY));
    }

    Vector3f MapTerrain::tileCoordinateToWorldCorner(int x, int y) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto worldX = (x * TileWidthInWorldUnits) - (widthInWorldUnits / 2.0f);
        auto worldY = (y * TileHeightInWorldUnits) - (heightInWorldUnits / 2.0f);

        return Vector3f(worldX, 0.0f, worldY);
    }

    Point MapTerrain::worldToHeightmapCoordinate(const Vector3f& position) const
    {
        auto heightPos = worldToHeightmapSpace(position);
        return Point(static_cast<int>(heightPos.x), static_cast<int>(heightPos.z));
    }

    Vector3f MapTerrain::heightmapIndexToWorldCorner(int x, int y) const
    {
        return heightmapToWorldSpace(Vector3f(x, 0.0f, y));
    }

    Vector3f MapTerrain::heightmapIndexToWorldCorner(Point p) const
    {
        return heightmapIndexToWorldCorner(p.x, p.y);
    }

    Vector3f MapTerrain::heightmapIndexToWorldCenter(int x, int y) const
    {
        return heightmapToWorldSpace(Vector3f(static_cast<float>(x) + 0.5f, 0.0f, static_cast<float>(y) + 0.5f));
    }

    Vector3f MapTerrain::heightmapIndexToWorldCenter(Point p) const
    {
        return heightmapIndexToWorldCenter(p.x, p.y);
    }

    Vector3f MapTerrain::worldToHeightmapSpace(const Vector3f& v) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();
        auto heightX = (v.x - (widthInWorldUnits / 2.0f)) / HeightTileWidthInWorldUnits;
        auto heightZ = (v.z - (heightInWorldUnits / 2.0f)) / HeightTileHeightInWorldUnits;

        return Vector3f(heightX, v.y, heightZ);
    }

    Vector3f MapTerrain::heightmapToWorldSpace(const Vector3f& v) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();
        auto worldX = (v.x * HeightTileWidthInWorldUnits) - (widthInWorldUnits / 2.0f);
        auto worldZ = (v.z * HeightTileHeightInWorldUnits) - (heightInWorldUnits / 2.0f);

        return Vector3f(worldX, v.y, worldZ);
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

    const Grid<unsigned char>& MapTerrain::getHeightMap() const
    {
        return heights;
    }

    float MapTerrain::getWidthInWorldUnits() const
    {
        return tiles.getWidth() * TileWidthInWorldUnits;
    }

    float MapTerrain::getHeightInWorldUnits() const
    {
        return tiles.getHeight() * TileHeightInWorldUnits;
    }

    Vector3f MapTerrain::topLeftCoordinateToWorld(const Vector3f& pos) const
    {
        return Vector3f(
            pos.x - (getWidthInWorldUnits() / 2.0f),
            pos.y,
            pos.z - (getHeightInWorldUnits() / 2.0f));
    }

    float MapTerrain::getHeightAt(float x, float z)
    {
        auto tileX = x / HeightTileWidthInWorldUnits;
        auto tileZ = z / HeightTileHeightInWorldUnits;
        if (
            tileX < 0.0f
            || tileX >= heights.getWidth() - 1
            || tileZ < 0.0f
            || tileZ >= heights.getHeight() - 1)
        {
            return 0.0f;
        }

        Line3f line(Vector3f(x, MaxHeight, z), Vector3f(x, MinHeight, z));
        auto pos = intersectLine(line);
        if (!pos)
        {
            return 0.0f;
        }
        return pos->y;
    }

    boost::optional<Vector3f> MapTerrain::intersectLine(const Line3f& line) const
    {
        auto startCell = worldToHeightmapCoordinate(line.start);

        auto ray = Ray3f::fromLine(line);

        int xDirection = ray.direction.x > 0 ? 1 : -1;
        int zDirection = ray.direction.z > 1 ? 1 : -1;
        float xPlaneOffset = (HeightTileWidthInWorldUnits / 2.0f) * xDirection;
        float zPlaneOffset = (HeightTileHeightInWorldUnits / 2.0f) * zDirection;

        while (true)
        {
            if (startCell.x >= 0
                && startCell.y >= 0
                && startCell.x < heights.getWidth() - 1
                && startCell.y < heights.getHeight() - 1)
            {
                auto cellIntersect = intersectWithHeightmapCell(line, startCell.x, startCell.y);
                if (cellIntersect)
                {
                    return *cellIntersect;
                }
            }

            auto cellPos = heightmapIndexToWorldCenter(startCell);

            Plane3f xPlane(Vector3f(cellPos.x + xPlaneOffset, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f));
            float xIntersect = xPlane.intersect(ray).get_value_or(std::numeric_limits<float>::infinity());

            Plane3f zPlane(Vector3f(0.0f, 0.0f, cellPos.z + zPlaneOffset), Vector3f(0.0f, 0.0f, 1.0f));
            float zIntersect = zPlane.intersect(ray).get_value_or(std::numeric_limits<float>::infinity());

            if (xIntersect < zIntersect)
            {
                if (xIntersect > 1.0f)
                {
                    return boost::none;
                }

                startCell.x += xDirection;
            }
            else
            {
                if (zIntersect > 1.0f)
                {
                    return boost::none;
                }

                startCell.y += zDirection;
            }
        }
    }

    boost::optional<Vector3f> MapTerrain::intersectWithHeightmapCell(const Line3f& line, int x, int y) const
    {
        auto posTopLeft = heightmapIndexToWorldCorner(x, y);
        posTopLeft.y = heights.get(x, y);

        auto posTopRight = heightmapIndexToWorldCorner(x + 1, y);
        posTopRight.y = heights.get(x + 1, y);

        auto posBottomLeft = heightmapIndexToWorldCorner(x, y + 1);
        posBottomLeft.y = heights.get(x, y + 1);

        auto posBottomRight = heightmapIndexToWorldCorner(x + 1, y + 1);
        posBottomRight.y = heights.get(x + 1, y + 1);

        float midHeight = (posTopLeft.y + posTopRight.y + posBottomLeft.y + posBottomRight.y) / 4.0f;
        auto posMiddle = heightmapIndexToWorldCenter(x, y);
        posMiddle.y = midHeight;

        Triangle3f left, bottom, right, top;

        // For robust collision testing under floating point arithmetic,
        // we ensure that the direction of any edge shared by two triangles
        // is the same in both triangles.
        // When this is true, the intersectLine intersection test
        // guarantees that a line passing exactly through the edge
        // will intersect at least one of the triangles.
        if (std::abs(y - x) % 2 == 0)  // checkerboard pattern
        {
            left = Triangle3f(posTopLeft, posMiddle, posBottomLeft);
            bottom = Triangle3f(posBottomLeft, posBottomRight, posMiddle);
            right = Triangle3f(posBottomRight, posMiddle, posTopRight);
            top = Triangle3f(posTopRight, posTopLeft, posMiddle);
        }
        else
        {
            left = Triangle3f(posTopLeft, posBottomLeft, posMiddle);
            bottom = Triangle3f(posBottomLeft, posMiddle, posBottomRight);
            right = Triangle3f(posBottomRight, posTopRight, posMiddle);
            top = Triangle3f(posTopRight, posMiddle, posTopLeft);
        }

        auto result = left.intersectLine(line);
        result = closestTo(line.start, result, bottom.intersectLine(line));
        result = closestTo(line.start, result, right.intersectLine(line));
        result = closestTo(line.start, result, top.intersectLine(line));

        return result;
    }
}
