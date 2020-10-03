#pragma once

#include <rwe/Grid.h>
#include <rwe/Point.h>
#include <rwe/camera/CabinetCamera.h>
#include <rwe/geometry/Line3f.h>
#include <rwe/render/TextureRegion.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <vector>

namespace rwe
{
    class MapTerrain
    {
    public:
        static constexpr SimScalar HeightTileWidthInWorldUnits = 16_ss;
        static constexpr SimScalar HeightTileHeightInWorldUnits = 16_ss;

        static constexpr SimScalar MaxHeight = 255_ss;
        static constexpr SimScalar MinHeight = 0_ss;

    private:
        Grid<unsigned char> heights;

        SimScalar seaLevel;

    public:
        MapTerrain(
            Grid<unsigned char>&& heights,
            SimScalar seaLevel);

        Point worldToHeightmapCoordinate(const SimVector& position) const;

        Point worldToHeightmapCoordinateNearest(const SimVector& position) const;

        SimVector heightmapIndexToWorldCorner(int x, int y) const;

        SimVector heightmapIndexToWorldCorner(Point p) const;

        SimVector heightmapIndexToWorldCenter(int x, int y) const;

        SimVector heightmapIndexToWorldCenter(std::size_t x, std::size_t y) const;

        SimVector heightmapIndexToWorldCenter(Point p) const;

        SimVector worldToHeightmapSpace(const SimVector& v) const;

        SimVector heightmapToWorldSpace(const SimVector& v) const;

        SimVector topLeftCoordinateToWorld(const SimVector& position) const;


        const Grid<unsigned char>& getHeightMap() const;

        SimScalar leftInWorldUnits() const;
        SimScalar rightCutoffInWorldUnits() const;
        SimScalar topInWorldUnits() const;
        SimScalar bottomCutoffInWorldUnits() const;

        SimScalar getWidthInWorldUnits() const;
        SimScalar getHeightInWorldUnits() const;

        SimScalar getHalfWidthInWorldUnits() const;
        SimScalar getHalfHeightInWorldUnits() const;

        /**
         * Gets the height of the terrain at the given world coordinates.
         * If the input is outside the heightmap grid, returns 0.
         */
        SimScalar getHeightAt(SimScalar x, SimScalar z) const;

        /**
         * Gets the height of the terrain at the given world coordinates.
         * If the input is outside the heightmap grid, returns None.
         */
        std::optional<SimScalar> tryGetHeightAt(SimScalar x, SimScalar z) const;

        std::optional<SimVector> intersectLine(const Line3x<SimScalar>& line) const;

        std::optional<SimVector> intersectWithHeightmapCell(const Line3x<SimScalar>& line, int x, int y) const;

        SimScalar getSeaLevel() const;

    private:
        bool isInHeightMapBounds(int x, int y) const;
    };
}
