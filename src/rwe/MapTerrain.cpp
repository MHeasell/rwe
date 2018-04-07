#include "MapTerrain.h"
#include <cmath>
#include <rwe/geometry/Plane3f.h>
#include <rwe/geometry/Triangle3f.h>

namespace rwe
{
    MapTerrain::MapTerrain(
        std::vector<TextureRegion>&& tileGraphics,
        Grid<size_t>&& tiles,
        Grid<unsigned char>&& heights,
        float seaLevel)
        : tileGraphics(std::move(tileGraphics)), tiles(std::move(tiles)), heights(std::move(heights)), seaLevel(seaLevel)
    {
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

    Point MapTerrain::worldToHeightmapCoordinateNearest(const Vector3f& position) const
    {
        auto heightPos = worldToHeightmapSpace(position);
        return Point(static_cast<int>(std::round(heightPos.x)), static_cast<int>(std::round(heightPos.z)));
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
        auto heightX = (v.x + (widthInWorldUnits / 2.0f)) / HeightTileWidthInWorldUnits;
        auto heightZ = (v.z + (heightInWorldUnits / 2.0f)) / HeightTileHeightInWorldUnits;

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

    float MapTerrain::getHeightAt(float x, float z) const
    {
        auto tilePos = worldToHeightmapCoordinate(Vector3f(x, 0.0f, z));
        if (
            tilePos.x < 0
            || static_cast<std::size_t>(tilePos.x) >= heights.getWidth() - 1
            || tilePos.y < 0
            || static_cast<std::size_t>(tilePos.y) >= heights.getHeight() - 1)
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

    std::optional<Vector3f> MapTerrain::intersectLine(const Line3f& line) const
    {
        auto heightmapPosition = worldToHeightmapSpace(line.start);
        Point startCell(static_cast<int>(heightmapPosition.x), static_cast<int>(heightmapPosition.z));

        auto ray = Ray3f::fromLine(line);

        int xDirection = ray.direction.x > 0 ? 1 : -1;
        int zDirection = ray.direction.z > 0 ? 1 : -1;
        float xPlaneOffset = (HeightTileWidthInWorldUnits / 2.0f) * xDirection;
        float zPlaneOffset = (HeightTileHeightInWorldUnits / 2.0f) * zDirection;

        while (true)
        {
            auto neighbourX = (heightmapPosition.x - startCell.x) > 0.5 ? 1 : -1;
            auto neighbourY = (heightmapPosition.z - startCell.y) > 0.5 ? 1 : -1;

            // Due to floating-point precision issues,
            // when the line passes very close to a cell boundary
            // the "intersectWithHeightmapCell" check may fail,
            // evaluating the line as passing outside the cell,
            // even if the line never leaves the cell according to the values of "startCell".
            // To guard against this, we check collision with the closest four cells,
            // ensuring that the line cannot escape through any "cracks".
            if (isInHeightMapBounds(startCell.x, startCell.y))
            {
                auto cellIntersect = intersectWithHeightmapCell(line, startCell.x, startCell.y);
                if (cellIntersect)
                {
                    return *cellIntersect;
                }
            }

            if (isInHeightMapBounds(startCell.x + neighbourX, startCell.y))
            {
                auto cellIntersect = intersectWithHeightmapCell(line, startCell.x + neighbourX, startCell.y);
                if (cellIntersect)
                {
                    return *cellIntersect;
                }
            }

            if (isInHeightMapBounds(startCell.x, startCell.y + neighbourY))
            {
                auto cellIntersect = intersectWithHeightmapCell(line, startCell.x, startCell.y + neighbourY);
                if (cellIntersect)
                {
                    return *cellIntersect;
                }
            }

            if (isInHeightMapBounds(startCell.x + neighbourX, startCell.y + neighbourY))
            {
                auto cellIntersect = intersectWithHeightmapCell(line, startCell.x + neighbourX, startCell.y + neighbourY);
                if (cellIntersect)
                {
                    return *cellIntersect;
                }
            }

            auto cellPos = heightmapIndexToWorldCenter(startCell);

            Plane3f xPlane(Vector3f(cellPos.x + xPlaneOffset, 0.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f));
            float xIntersect = xPlane.intersect(ray).value_or(std::numeric_limits<float>::infinity());

            Plane3f zPlane(Vector3f(0.0f, 0.0f, cellPos.z + zPlaneOffset), Vector3f(0.0f, 0.0f, 1.0f));
            float zIntersect = zPlane.intersect(ray).value_or(std::numeric_limits<float>::infinity());

            if (xIntersect < zIntersect)
            {
                if (xIntersect > 1.0f)
                {
                    return std::nullopt;
                }

                startCell.x += xDirection;
                heightmapPosition.x += xDirection;
            }
            else
            {
                if (zIntersect > 1.0f)
                {
                    return std::nullopt;
                }

                startCell.y += zDirection;
                heightmapPosition.z += zDirection;
            }
        }
    }

    std::optional<Vector3f> MapTerrain::intersectWithHeightmapCell(const Line3f& line, int x, int y) const
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
        if (std::abs(y - x) % 2 == 0) // checkerboard pattern
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

    float MapTerrain::getSeaLevel() const
    {
        return seaLevel;
    }

    bool MapTerrain::isInHeightMapBounds(int x, int y) const
    {
        return x >= 0
            && y >= 0
            && static_cast<std::size_t>(x) < heights.getWidth() - 1
            && static_cast<std::size_t>(y) < heights.getHeight() - 1;
    }
}
