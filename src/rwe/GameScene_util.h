#pragma once

#include <rwe/RenderService.h>
#include <rwe/math/Matrix4x.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/sim/MapTerrain.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitMesh.h>
#include <rwe/sim/UnitPieceDefinition.h>
#include <vector>

namespace rwe
{
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const std::vector<UnitPieceDefinition>& pieceDefinitions, const std::vector<UnitMesh>& pieces);

    void
    drawPathfindingVisualisation(const MapTerrain& terrain, const AStarPathInfo<Point, PathCost>& pathInfo, ColoredMeshBatch& batch);

    void
    drawTerrainArrow(const MapTerrain& terrain, const Point& start, const Point& end, const Color& color, ColoredMeshBatch& batch);

    void drawOccupiedGrid(const CabinetCamera& camera, const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, ColoredMeshBatch& batch);

    void drawMovementClassCollisionGrid(const MapTerrain& terrain, const Grid<char>& movementClassGrid, const CabinetCamera& camera, ColoredMeshBatch& batch);
}
