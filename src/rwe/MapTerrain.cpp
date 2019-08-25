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
        SimScalar seaLevel)
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

    Point MapTerrain::worldToTileCoordinate(const SimVector& position) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto newX = (position.x + (widthInWorldUnits / 2_ss)) / TileWidthInWorldUnits;
        auto newY = (position.z + (heightInWorldUnits / 2_ss)) / TileHeightInWorldUnits;

        return Point(static_cast<int>(newX.value), static_cast<int>(newY.value));
    }

    SimVector MapTerrain::tileCoordinateToWorldCorner(int x, int y) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();

        auto worldX = (SimScalar(x) * TileWidthInWorldUnits) - (widthInWorldUnits / 2_ss);
        auto worldY = (SimScalar(y) * TileHeightInWorldUnits) - (heightInWorldUnits / 2_ss);

        return SimVector(worldX, 0_ss, worldY);
    }

    Point MapTerrain::worldToHeightmapCoordinate(const SimVector& position) const
    {
        auto heightPos = worldToHeightmapSpace(position);
        return Point(static_cast<int>(heightPos.x.value), static_cast<int>(heightPos.z.value));
    }

    Point MapTerrain::worldToHeightmapCoordinateNearest(const SimVector& position) const
    {
        auto heightPos = worldToHeightmapSpace(position);
        return Point(roundToInt(heightPos.x), roundToInt(heightPos.z));
    }

    SimVector MapTerrain::heightmapIndexToWorldCorner(int x, int y) const
    {
        return heightmapToWorldSpace(SimVector(SimScalar(x), 0_ss, SimScalar(y)));
    }

    SimVector MapTerrain::heightmapIndexToWorldCorner(Point p) const
    {
        return heightmapIndexToWorldCorner(p.x, p.y);
    }

    SimVector MapTerrain::heightmapIndexToWorldCenter(int x, int y) const
    {
        return heightmapToWorldSpace(SimVector(SimScalar(x) + 0.5_ssf, 0_ss, SimScalar(y) + 0.5_ssf));
    }

    SimVector MapTerrain::heightmapIndexToWorldCenter(std::size_t x, std::size_t y) const
    {
        return heightmapToWorldSpace(SimVector(SimScalar(x) + 0.5_ssf, 0_ss, SimScalar(y) + 0.5_ssf));
    }

    SimVector MapTerrain::heightmapIndexToWorldCenter(Point p) const
    {
        return heightmapIndexToWorldCenter(p.x, p.y);
    }

    SimVector MapTerrain::worldToHeightmapSpace(const SimVector& v) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();
        auto heightX = (v.x + (widthInWorldUnits / 2_ss)) / HeightTileWidthInWorldUnits;
        auto heightZ = (v.z + (heightInWorldUnits / 2_ss)) / HeightTileHeightInWorldUnits;

        return SimVector(heightX, v.y, heightZ);
    }

    SimVector MapTerrain::heightmapToWorldSpace(const SimVector& v) const
    {
        auto widthInWorldUnits = getWidthInWorldUnits();
        auto heightInWorldUnits = getHeightInWorldUnits();
        auto worldX = (v.x * HeightTileWidthInWorldUnits) - (widthInWorldUnits / 2_ss);
        auto worldZ = (v.z * HeightTileHeightInWorldUnits) - (heightInWorldUnits / 2_ss);

        return SimVector(worldX, v.y, worldZ);
    }

    SimScalar MapTerrain::leftInWorldUnits() const
    {
        return -((SimScalar(tiles.getWidth()) / 2_ss) * TileWidthInWorldUnits);
    }

    SimScalar MapTerrain::rightCutoffInWorldUnits() const
    {
        auto right = (SimScalar(tiles.getWidth()) / 2_ss) * TileWidthInWorldUnits;
        return right - TileWidthInWorldUnits;
    }

    SimScalar MapTerrain::topInWorldUnits() const
    {
        return -((SimScalar(tiles.getHeight()) / 2_ss) * TileHeightInWorldUnits);
    }

    SimScalar MapTerrain::bottomCutoffInWorldUnits() const
    {
        auto bottom = (SimScalar(tiles.getHeight()) / 2_ss) * TileHeightInWorldUnits;
        return bottom - (TileHeightInWorldUnits * 4_ss);
    }

    const Grid<unsigned char>& MapTerrain::getHeightMap() const
    {
        return heights;
    }

    SimScalar MapTerrain::getWidthInWorldUnits() const
    {
        return SimScalar(tiles.getWidth()) * TileWidthInWorldUnits;
    }

    SimScalar MapTerrain::getHeightInWorldUnits() const
    {
        return SimScalar(tiles.getHeight()) * TileHeightInWorldUnits;
    }

    SimVector MapTerrain::topLeftCoordinateToWorld(const SimVector& pos) const
    {
        return SimVector(
            pos.x - (getWidthInWorldUnits() / 2_ss),
            pos.y,
            pos.z - (getHeightInWorldUnits() / 2_ss));
    }

    SimScalar MapTerrain::getHeightAt(SimScalar x, SimScalar z) const
    {
        auto tilePos = worldToHeightmapCoordinate(SimVector(x, 0_ss, z));
        if (
            tilePos.x < 0
            || static_cast<std::size_t>(tilePos.x) >= heights.getWidth() - 1
            || tilePos.y < 0
            || static_cast<std::size_t>(tilePos.y) >= heights.getHeight() - 1)
        {
            return 0_ss;
        }

        Line3x<SimScalar> line(SimVector(x, MaxHeight, z), SimVector(x, MinHeight, z));
        auto pos = intersectLine(line);
        if (!pos)
        {
            return 0_ss;
        }
        return pos->y;
    }

    std::optional<SimVector> MapTerrain::intersectLine(const Line3x<SimScalar>& line) const
    {
        auto heightmapPosition = worldToHeightmapSpace(line.start);
        Point startCell(static_cast<int>(heightmapPosition.x.value), static_cast<int>(heightmapPosition.z.value));

        auto ray = Ray3x<SimScalar>::fromLine(line);

        auto xDirection = ray.direction.x > 0_ss ? 1 : -1;
        auto zDirection = ray.direction.z > 0_ss ? 1 : -1;
        auto xPlaneOffset = (HeightTileWidthInWorldUnits / 2_ss) * SimScalar(xDirection);
        auto zPlaneOffset = (HeightTileHeightInWorldUnits / 2_ss) * SimScalar(zDirection);

        while (true)
        {
            auto neighbourX = (heightmapPosition.x - SimScalar(startCell.x)) > 0.5_ssf ? 1 : -1;
            auto neighbourY = (heightmapPosition.z - SimScalar(startCell.y)) > 0.5_ssf ? 1 : -1;

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

            Plane3x<SimScalar> xPlane(SimVector(cellPos.x + xPlaneOffset, 0_ss, 0_ss), SimVector(1_ss, 0_ss, 0_ss));
            auto xIntersect = xPlane.intersect(ray);

            Plane3x<SimScalar> zPlane(SimVector(0_ss, 0_ss, cellPos.z + zPlaneOffset), SimVector(0_ss, 0_ss, 1_ss));
            auto zIntersect = zPlane.intersect(ray);

            if (!xIntersect && !zIntersect)
            {
                return std::nullopt;
            }

            if (xIntersect && (!zIntersect || *xIntersect < *zIntersect))
            {
                if (*xIntersect > 1_ss)
                {
                    return std::nullopt;
                }

                startCell.x += xDirection;
                heightmapPosition.x += SimScalar(xDirection);
            }
            else
            {
                if (*zIntersect > 1_ss)
                {
                    return std::nullopt;
                }

                startCell.y += zDirection;
                heightmapPosition.z += SimScalar(zDirection);
            }
        }
    }

    std::optional<SimVector> MapTerrain::intersectWithHeightmapCell(const Line3x<SimScalar>& line, int x, int y) const
    {
        auto posTopLeft = heightmapIndexToWorldCorner(x, y);
        posTopLeft.y = SimScalar(heights.get(x, y));

        auto posTopRight = heightmapIndexToWorldCorner(x + 1, y);
        posTopRight.y = SimScalar(heights.get(x + 1, y));

        auto posBottomLeft = heightmapIndexToWorldCorner(x, y + 1);
        posBottomLeft.y = SimScalar(heights.get(x, y + 1));

        auto posBottomRight = heightmapIndexToWorldCorner(x + 1, y + 1);
        posBottomRight.y = SimScalar(heights.get(x + 1, y + 1));

        auto midHeight = (posTopLeft.y + posTopRight.y + posBottomLeft.y + posBottomRight.y) / 4_ss;
        auto posMiddle = heightmapIndexToWorldCenter(x, y);
        posMiddle.y = midHeight;

        Triangle3x<SimScalar> left, bottom, right, top;

        // For robust collision testing under floating point arithmetic,
        // we ensure that the direction of any edge shared by two triangles
        // is the same in both triangles.
        // When this is true, the intersectLine intersection test
        // guarantees that a line passing exactly through the edge
        // will intersect at least one of the triangles.
        if (std::abs(y - x) % 2 == 0) // checkerboard pattern
        {
            left = Triangle3x<SimScalar>(posTopLeft, posMiddle, posBottomLeft);
            bottom = Triangle3x<SimScalar>(posBottomLeft, posBottomRight, posMiddle);
            right = Triangle3x<SimScalar>(posBottomRight, posMiddle, posTopRight);
            top = Triangle3x<SimScalar>(posTopRight, posTopLeft, posMiddle);
        }
        else
        {
            left = Triangle3x<SimScalar>(posTopLeft, posBottomLeft, posMiddle);
            bottom = Triangle3x<SimScalar>(posBottomLeft, posMiddle, posBottomRight);
            right = Triangle3x<SimScalar>(posBottomRight, posTopRight, posMiddle);
            top = Triangle3x<SimScalar>(posTopRight, posMiddle, posTopLeft);
        }

        auto result = left.intersectLine(line);
        result = closestTo(line.start, result, bottom.intersectLine(line));
        result = closestTo(line.start, result, right.intersectLine(line));
        result = closestTo(line.start, result, top.intersectLine(line));

        return result;
    }

    SimScalar MapTerrain::getSeaLevel() const
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
