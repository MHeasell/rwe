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
    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const UnitModelDefinition& modelDefinition, const std::vector<UnitMesh>& pieces);

    void
    drawPathfindingVisualisation(const MapTerrain& terrain, const AStarPathInfo<Point, PathCost>& pathInfo, ColoredMeshBatch& batch);

    void
    drawTerrainArrow(const MapTerrain& terrain, const Point& start, const Point& end, const Color& color, ColoredMeshBatch& batch);

    void drawOccupiedGrid(const Vector3f& cameraPosition, float viewportWidth, float viewportHeight, const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, ColoredMeshBatch& batch);

    void drawMovementClassCollisionGrid(const MapTerrain& terrain, const Grid<char>& movementClassGrid, const Vector3f& cameraPosition, float viewportWidth, float viewportHeight, ColoredMeshBatch& batch);

    void drawUnit(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const Unit& unit,
        PlayerColorIndex playerColorIndex,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch);

    void drawMeshFeature(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const MapFeature& feature,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch);

    void drawUnitShadow(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const Unit& unit,
        float frac,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch);

    void drawFeatureMeshShadow(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const MapFeature& feature,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch);

    void drawFeature(
        const UnitDatabase& unitDatabase,
        const MeshDatabase& meshDatabase,
        const MapFeature& feature,
        const Matrix4f& viewProjectionMatrix,
        SpriteBatch& batch);
    void drawFeatureShadow(
        const UnitDatabase& unitDatabase,
        const MeshDatabase& meshDatabase,
        const MapFeature& feature,
        const Matrix4f& viewProjectionMatrix,
        SpriteBatch& batch);

    void drawNanoLine(const Vector3f& start, const Vector3f& end, ColoredMeshBatch& batch);

    void drawParticle(const MeshDatabase& meshDatabase, GameTime currentTime, const Matrix4f& viewProjectionMatrix, const Particle& particle, SpriteBatch& batch);

    void updateParticles(const MeshDatabase& meshDatabase, GameTime currentTime, std::vector<Particle>& particles);
}
