#include "GameScene_util.h"

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <rwe/Index.h>

namespace rwe
{
    class IsOccupiedVisitor
    {
    public:
        bool operator()(const OccupiedNone&) const
        {
            return false;
        }

        bool operator()(const OccupiedUnit&) const
        {
            return true;
        }

        bool operator()(const OccupiedFeature&) const
        {
            return true;
        }
    };

    Vector3f colorToVector3f(const Color& color)
    {
        return Vector3f(static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f, static_cast<float>(color.b) / 255.0f);
    }

    void pushTriangle(std::vector<GlColoredVertex>& vs, const Triangle3f& tri, const Vector3f& color)
    {
        vs.emplace_back(tri.a, color);
        vs.emplace_back(tri.b, color);
        vs.emplace_back(tri.c, color);
    }

    void pushTriangle(std::vector<GlColoredVertex>& vs, const Triangle3f& tri)
    {
        pushTriangle(vs, tri, Vector3f(1.0f, 1.0f, 1.0f));
    }

    void pushTriangle(std::vector<GlColoredVertex>& vs, const Vector3f& a, const Vector3f& b, const Vector3f& c)
    {
        pushTriangle(vs, Triangle3f(a, b, c));
    }

    void pushTriangle(std::vector<GlColoredVertex>& vs, const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& color)
    {
        pushTriangle(vs, Triangle3f(a, b, c), color);
    }

    void pushLine(std::vector<GlColoredVertex>& vs, const Line3f& line, const Vector3f& color)
    {
        vs.emplace_back(line.start, color);
        vs.emplace_back(line.end, color);
    }

    void pushLine(std::vector<GlColoredVertex>& vs, const Line3f& line)
    {
        vs.emplace_back(line.start, Vector3f(1.0f, 1.0f, 1.0f));
        vs.emplace_back(line.end, Vector3f(1.0f, 1.0f, 1.0f));
    }

    void pushLine(std::vector<GlColoredVertex>& vs, const Vector3f& start, const Vector3f& end)
    {
        pushLine(vs, Line3f(start, end));
    }

    void pushLine(std::vector<GlColoredVertex>& vs, const Vector3f& start, const Vector3f& end, const Vector3f& color)
    {
        pushLine(vs, Line3f(start, end), color);
    }

    void
    drawPathfindingVisualisation(const MapTerrain& terrain, const AStarPathInfo<Point, PathCost>& pathInfo, ColoredMeshBatch& batch)
    {
        for (const auto& item : pathInfo.closedVertices)
        {
            if (!item.second.predecessor)
            {
                continue;
            }

            auto start = (*item.second.predecessor)->vertex;
            auto end = item.second.vertex;
            drawTerrainArrow(terrain, start, end, Color(255, 0, 0), batch);
        }

        if (pathInfo.path.size() > 1)
        {
            for (auto it = pathInfo.path.begin() + 1; it != pathInfo.path.end(); ++it)
            {
                auto start = *(it - 1);
                auto end = *it;
                drawTerrainArrow(terrain, start, end, Color(0, 0, 255), batch);
            }
        }
    }

    void
    drawTerrainArrow(const MapTerrain& terrain, const Point& start, const Point& end, const Color& color, ColoredMeshBatch& batch)
    {
        auto worldStart = terrain.heightmapIndexToWorldCenter(start);
        worldStart.y = terrain.getHeightAt(worldStart.x, worldStart.z);
        auto worldStartF = simVectorToFloat(worldStart);

        auto worldEnd = terrain.heightmapIndexToWorldCenter(end);
        worldEnd.y = terrain.getHeightAt(worldEnd.x, worldEnd.z);
        auto worldEndF = simVectorToFloat(worldEnd);

        auto armTemplate = (worldStartF - worldEndF).normalized() * 6.0f;
        auto arm1 = Matrix4f::translation(worldEndF) * Matrix4f::rotationY(Pif / 6.0f) * armTemplate;
        auto arm2 = Matrix4f::translation(worldEndF) * Matrix4f::rotationY(-Pif / 6.0f) * armTemplate;

        auto floatColor = colorToVector3f(color);

        pushLine(batch.lines, worldStartF, worldEndF, floatColor);
        pushLine(batch.lines, worldEndF, arm1, floatColor);
        pushLine(batch.lines, worldEndF, arm2, floatColor);
    }

    void drawOccupiedGrid(const Vector3f& cameraPosition, float viewportWidth, float viewportHeight, const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, ColoredMeshBatch& batch)
    {
        auto halfWidth = viewportWidth / 2.0f;
        auto halfHeight = viewportHeight / 2.0f;
        auto left = cameraPosition.x - halfWidth;
        auto top = cameraPosition.z - halfHeight;
        auto right = cameraPosition.x + halfWidth;
        auto bottom = cameraPosition.z + halfHeight;

        assert(left < right);
        assert(top < bottom);

        assert(terrain.getHeightMap().getWidth() >= 2);
        assert(terrain.getHeightMap().getHeight() >= 2);

        auto topLeftCell = terrain.worldToHeightmapCoordinate(floatToSimVector(Vector3f(left, 0.0f, top)));
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(occupiedGrid.getWidth() - 1));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(occupiedGrid.getHeight() - 1));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(floatToSimVector(Vector3f(right, 0.0f, bottom)));
        bottomRightCell.y += 7; // compensate for height
        bottomRightCell.x = std::clamp(bottomRightCell.x, 0, static_cast<int>(occupiedGrid.getWidth() - 1));
        bottomRightCell.y = std::clamp(bottomRightCell.y, 0, static_cast<int>(occupiedGrid.getHeight() - 1));

        assert(topLeftCell.x <= bottomRightCell.x);
        assert(topLeftCell.y <= bottomRightCell.y);

        for (int y = topLeftCell.y; y <= bottomRightCell.y; ++y)
        {
            for (int x = topLeftCell.x; x <= bottomRightCell.x; ++x)
            {
                auto pos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x, y));
                pos.y = terrain.getHeightMap().get(x, y);

                auto rightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y));
                rightPos.y = terrain.getHeightMap().get(x + 1, y);

                auto downPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x, y + 1));
                downPos.y = terrain.getHeightMap().get(x, y + 1);

                pushLine(batch.lines, pos, rightPos);
                pushLine(batch.lines, pos, downPos);

                const auto& cell = occupiedGrid.get(x, y);
                if (std::visit(IsOccupiedVisitor(), cell.occupiedType))
                {
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);

                    pushTriangle(batch.triangles, pos, downPos, downRightPos);
                    pushTriangle(batch.triangles, pos, downRightPos, rightPos);
                }

                const auto insetAmount = 4.0f;

                if (cell.buildingCell && !cell.buildingCell->passable)
                {
                    auto topLeftPos = pos + Vector3f(insetAmount, 0.0f, insetAmount);
                    auto topRightPos = rightPos + Vector3f(-insetAmount, 0.0f, insetAmount);
                    auto bottomLeftPos = downPos + Vector3f(insetAmount, 0.0f, -insetAmount);
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);
                    downRightPos += Vector3f(-insetAmount, 0.0f, -insetAmount);

                    pushTriangle(batch.triangles, topLeftPos, bottomLeftPos, downRightPos, Vector3f(1.0f, 0.0f, 0.0f));
                    pushTriangle(batch.triangles, topLeftPos, downRightPos, topRightPos, Vector3f(1.0f, 0.0f, 0.0f));
                }

                if (cell.buildingCell && cell.buildingCell->passable)
                {
                    auto topLeftPos = pos + Vector3f(insetAmount, 0.0f, insetAmount);
                    auto topRightPos = rightPos + Vector3f(-insetAmount, 0.0f, insetAmount);
                    auto bottomLeftPos = downPos + Vector3f(insetAmount, 0.0f, -insetAmount);
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);
                    downRightPos += Vector3f(-insetAmount, 0.0f, -insetAmount);

                    pushTriangle(batch.triangles, topLeftPos, bottomLeftPos, downRightPos, Vector3f(0.0f, 1.0f, 0.0f));
                    pushTriangle(batch.triangles, topLeftPos, downRightPos, topRightPos, Vector3f(0.0f, 1.0f, 0.0f));
                }
            }
        }
    }

    void drawMovementClassCollisionGrid(
        const MapTerrain& terrain,
        const Grid<char>& movementClassGrid,
        const Vector3f& cameraPosition,
        float viewportWidth,
        float viewportHeight,
        ColoredMeshBatch& batch)
    {
        auto halfWidth = viewportWidth / 2.0f;
        auto halfHeight = viewportHeight / 2.0f;
        auto left = cameraPosition.x - halfWidth;
        auto top = cameraPosition.z - halfHeight;
        auto right = cameraPosition.x + halfWidth;
        auto bottom = cameraPosition.z + halfHeight;

        assert(left < right);
        assert(top < bottom);

        assert(terrain.getHeightMap().getWidth() >= 2);
        assert(terrain.getHeightMap().getHeight() >= 2);
        assert(movementClassGrid.getWidth() <= terrain.getHeightMap().getWidth());
        assert(movementClassGrid.getHeight() <= terrain.getHeightMap().getHeight());

        auto topLeftCell = terrain.worldToHeightmapCoordinate(floatToSimVector(Vector3f(left, 0.0f, top)));
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(movementClassGrid.getWidth() - 1));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(movementClassGrid.getHeight() - 1));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(floatToSimVector(Vector3f(right, 0.0f, bottom)));
        bottomRightCell.y += 7; // compensate for height
        bottomRightCell.x = std::clamp(bottomRightCell.x, 0, static_cast<int>(movementClassGrid.getWidth() - 1));
        bottomRightCell.y = std::clamp(bottomRightCell.y, 0, static_cast<int>(movementClassGrid.getHeight() - 1));

        assert(topLeftCell.x <= bottomRightCell.x);
        assert(topLeftCell.y <= bottomRightCell.y);

        for (int y = topLeftCell.y; y <= bottomRightCell.y; ++y)
        {
            for (int x = topLeftCell.x; x <= bottomRightCell.x; ++x)
            {
                auto pos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x, y));
                pos.y = terrain.getHeightMap().get(x, y);

                auto rightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y));
                rightPos.y = terrain.getHeightMap().get(x + 1, y);

                auto downPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x, y + 1));
                downPos.y = terrain.getHeightMap().get(x, y + 1);

                pushLine(batch.lines, pos, rightPos);
                pushLine(batch.lines, pos, downPos);

                if (!movementClassGrid.get(x, y))
                {
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);

                    pushTriangle(batch.triangles, pos, downPos, downRightPos);
                    pushTriangle(batch.triangles, pos, downRightPos, rightPos);
                }
            }
        }
    }

    Matrix4f getPieceTransformForRender(const std::string& pieceName, const UnitModelDefinition& modelDefinition, const std::vector<UnitMesh>& pieces, float frac)
    {
        assert(modelDefinition.pieces.size() == pieces.size());

        std::optional<std::string> parentPiece = pieceName;
        auto matrix = Matrix4f::identity();

        do
        {
            auto pieceIndexIt = modelDefinition.pieceIndicesByName.find(toUpper(*parentPiece));
            if (pieceIndexIt == modelDefinition.pieceIndicesByName.end())
            {
                throw std::runtime_error("missing piece definition: " + *parentPiece);
            }
            const auto& pieceDef = modelDefinition.pieces[pieceIndexIt->second];

            parentPiece = pieceDef.parent;

            auto pieceStateIt = pieces.begin() + pieceIndexIt->second;

            auto position = lerp(simVectorToFloat(pieceDef.origin + pieceStateIt->previousOffset), simVectorToFloat(pieceDef.origin + pieceStateIt->offset), frac);
            auto rotationX = angleLerp(toRadians(pieceStateIt->previousRotationX).value, toRadians(pieceStateIt->rotationX).value, frac);
            auto rotationY = angleLerp(toRadians(pieceStateIt->previousRotationY).value, toRadians(pieceStateIt->rotationY).value, frac);
            auto rotationZ = angleLerp(toRadians(pieceStateIt->previousRotationZ).value, toRadians(pieceStateIt->rotationZ).value, frac);
            matrix = Matrix4f::translation(position) * Matrix4f::rotationZXY(Vector3f(rotationX, rotationY, rotationZ)) * matrix;
        } while (parentPiece);

        return matrix;
    }

    Matrix4f getPieceTransformForRender(const std::string& pieceName, const UnitModelDefinition& modelDefinition)
    {
        std::optional<std::string> parentPiece = pieceName;
        auto pos = SimVector(0_ss, 0_ss, 0_ss);

        do
        {
            auto pieceIndexIt = modelDefinition.pieceIndicesByName.find(toUpper(*parentPiece));
            if (pieceIndexIt == modelDefinition.pieceIndicesByName.end())
            {
                throw std::runtime_error("missing piece definition: " + *parentPiece);
            }

            const auto& pieceDef = modelDefinition.pieces[pieceIndexIt->second];

            parentPiece = pieceDef.parent;

            pos += pieceDef.origin;
        } while (parentPiece);

        return Matrix4f::translation(simVectorToFloat(pos));
    }

    void drawShaderMesh(
        const Matrix4f& viewProjectionMatrix,
        const ShaderMesh& mesh,
        const Matrix4f& matrix,
        bool shaded,
        PlayerColorIndex playerColorIndex,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        std::vector<UnitTextureMeshRenderInfo>& batch)
    {
        auto mvpMatrix = viewProjectionMatrix * matrix;

        if (mesh.vertices)
        {
            batch.push_back(UnitTextureMeshRenderInfo{&*mesh.vertices, matrix, mvpMatrix, shaded, unitTextureAtlas});
        }
        if (mesh.teamVertices)
        {
            batch.push_back(UnitTextureMeshRenderInfo{&*mesh.teamVertices, matrix, mvpMatrix, shaded, unitTeamTextureAtlases.at(playerColorIndex.value).get()});
        }
    }

    void drawShaderMeshShadow(
        const Matrix4f& viewProjectionMatrix,
        const ShaderMesh& mesh,
        const Matrix4f& matrix,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        std::vector<UnitTextureShadowMeshRenderInfo>& batch)
    {
        if (mesh.vertices)
        {
            batch.push_back(UnitTextureShadowMeshRenderInfo{&*mesh.vertices, matrix, viewProjectionMatrix, unitTextureAtlas, groundHeight});
        }
        if (mesh.teamVertices)
        {
            batch.push_back(UnitTextureShadowMeshRenderInfo{&*mesh.teamVertices, matrix, viewProjectionMatrix, unitTeamTextureAtlases.at(0).get(), groundHeight});
        }
    }

    void drawUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const std::string& objectName,
        const std::vector<UnitMesh>& meshes,
        const Matrix4f& modelMatrix,
        PlayerColorIndex playerColorIndex,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }
        assert(modelDefinition->get().pieces.size() == meshes.size());

        for (Index i = 0; i < getSize(modelDefinition->get().pieces); ++i)
        {
            const auto& pieceDef = modelDefinition->get().pieces[i];
            const auto& mesh = meshes[i];
            if (!mesh.visible)
            {
                continue;
            }

            auto matrix = modelMatrix * getPieceTransformForRender(pieceDef.name, modelDefinition->get(), meshes, frac);

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value().get().mesh;
            drawShaderMesh(viewProjectionMatrix, resolvedMesh, matrix, mesh.shaded, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawUnitShadowMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const std::string& objectName,
        const std::vector<UnitMesh>& meshes,
        const Matrix4f& modelMatrix,
        float frac,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }
        assert(modelDefinition->get().pieces.size() == meshes.size());

        for (Index i = 0; i < getSize(modelDefinition->get().pieces); ++i)
        {
            const auto& pieceDef = modelDefinition->get().pieces[i];
            const auto& mesh = meshes[i];
            if (!mesh.visible)
            {
                continue;
            }

            auto matrix = modelMatrix * getPieceTransformForRender(pieceDef.name, modelDefinition->get(), meshes, frac);

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value().get().mesh;
            drawShaderMeshShadow(viewProjectionMatrix, resolvedMesh, matrix, groundHeight, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawUnitShadowMeshNoPieces(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const std::string& objectName,
        const Matrix4f& modelMatrix,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }

        for (Index i = 0; i < getSize(modelDefinition->get().pieces); ++i)
        {
            const auto& pieceDef = modelDefinition->get().pieces[i];

            auto matrix = modelMatrix * getPieceTransformForRender(pieceDef.name, modelDefinition->get());

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value().get().mesh;
            drawShaderMeshShadow(viewProjectionMatrix, resolvedMesh, matrix, groundHeight, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawBuildingShaderMesh(
        const Matrix4f& viewProjectionMatrix,
        const ShaderMesh& mesh,
        const Matrix4f& matrix,
        bool shaded,
        float percentComplete,
        float unitY,
        PlayerColorIndex playerColorIndex,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        std::vector<UnitBuildingMeshRenderInfo>& batch)
    {
        auto mvpMatrix = viewProjectionMatrix * matrix;
        if (mesh.vertices)
        {
            batch.push_back(UnitBuildingMeshRenderInfo{&*mesh.vertices, matrix, mvpMatrix, shaded, unitTextureAtlas, percentComplete, unitY});
        }
        if (mesh.teamVertices)
        {
            batch.push_back(UnitBuildingMeshRenderInfo{&*mesh.teamVertices, matrix, mvpMatrix, shaded, unitTeamTextureAtlases.at(playerColorIndex.value).get(), percentComplete, unitY});
        }
    }

    void drawBuildingUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const std::string& objectName,
        const std::vector<UnitMesh>& meshes,
        const Matrix4f& modelMatrix,
        float percentComplete,
        float unitY,
        PlayerColorIndex playerColorIndex,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }

        for (Index i = 0; i < getSize(modelDefinition->get().pieces); ++i)
        {
            const auto& pieceDef = modelDefinition->get().pieces[i];
            const auto& mesh = meshes[i];
            if (!mesh.visible)
            {
                continue;
            }

            auto matrix = modelMatrix * getPieceTransformForRender(pieceDef.name, modelDefinition->get(), meshes, frac);

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value().get().mesh;
            drawBuildingShaderMesh(viewProjectionMatrix, resolvedMesh, matrix, mesh.shaded, percentComplete, unitY, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.buildingMeshes);
        }
    }

    void drawProjectileUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const std::string& objectName,
        const Matrix4f& modelMatrix,
        PlayerColorIndex playerColorIndex,
        bool shaded,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }

        for (const auto& pieceDef : modelDefinition->get().pieces)
        {
            auto matrix = modelMatrix * getPieceTransformForRender(pieceDef.name, modelDefinition->get());
            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value().get().mesh;
            drawShaderMesh(viewProjectionMatrix, resolvedMesh, matrix, shaded, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawUnit(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const UnitState& unit,
        const UnitDefinition& unitDefinition,
        PlayerColorIndex playerColorIndex,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        auto position = lerp(simVectorToFloat(unit.previousPosition), simVectorToFloat(unit.position), frac);
        auto rotation = angleLerp(toRadians(unit.previousRotation).value, toRadians(unit.rotation).value, frac);
        auto transform = Matrix4f::translation(position) * Matrix4f::rotationY(rotation);
        if (unit.isBeingBuilt(unitDefinition))
        {
            drawBuildingUnitMesh(unitDatabase, meshDatabase, viewProjectionMatrix, unitDefinition.objectName, unit.pieces, transform, unit.getPreciseCompletePercent(unitDefinition), position.y, playerColorIndex, frac, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
        else
        {
            drawUnitMesh(unitDatabase, meshDatabase, viewProjectionMatrix, unitDefinition.objectName, unit.pieces, transform, playerColorIndex, frac, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
    }

    void drawMeshFeature(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const MapFeature& feature,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        const auto& featureMediaInfo = meshDatabase.getFeature(feature.featureName);

        if (auto objectInfo = std::get_if<FeatureObjectInfo>(&featureMediaInfo.renderInfo); objectInfo != nullptr)
        {
            auto matrix = Matrix4f::translation(simVectorToFloat(feature.position)) * Matrix4f::rotationY(toRadians(feature.rotation).value);
            drawProjectileUnitMesh(unitDatabase, meshDatabase, viewProjectionMatrix, objectInfo->objectName, matrix, PlayerColorIndex(0), true, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
    }

    void drawUnitShadow(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const UnitState& unit,
        const UnitDefinition& unitDefinition,
        float frac,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch)
    {
        auto position = lerp(simVectorToFloat(unit.previousPosition), simVectorToFloat(unit.position), frac);
        auto rotation = angleLerp(toRadians(unit.previousRotation).value, toRadians(unit.rotation).value, frac);
        auto transform = Matrix4f::translation(position) * Matrix4f::rotationY(rotation);

        drawUnitShadowMesh(unitDatabase, meshDatabase, viewProjectionMatrix, unitDefinition.objectName, unit.pieces, transform, frac, groundHeight, unitTextureAtlas, unitTeamTextureAtlases, batch);
    }

    void drawFeatureMeshShadow(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const MapFeature& feature,
        float groundHeight,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitShadowMeshBatch& batch)
    {
        const auto& featureMediaInfo = meshDatabase.getFeature(feature.featureName);

        auto objectInfo = std::get_if<FeatureObjectInfo>(&featureMediaInfo.renderInfo);
        if (objectInfo == nullptr)
        {
            return;
        }

        const auto& position = feature.position;
        auto matrix = Matrix4f::translation(simVectorToFloat(position)) * Matrix4f::rotationY(toRadians(feature.rotation).value);

        drawUnitShadowMeshNoPieces(unitDatabase, meshDatabase, viewProjectionMatrix, objectInfo->objectName, matrix, groundHeight, unitTextureAtlas, unitTeamTextureAtlases, batch);
    }

    void drawFeature(
        const UnitDatabase& unitDatabase,
        const MeshDatabase& meshDatabase,
        const MapFeature& feature,
        const Matrix4f& viewProjectionMatrix,
        SpriteBatch& batch)
    {
        const auto& featureDefinition = unitDatabase.getFeature(feature.featureName);
        const auto& featureMediaInfo = meshDatabase.getFeature(feature.featureName);

        auto spriteInfo = std::get_if<FeatureSpriteInfo>(&featureMediaInfo.renderInfo);
        if (spriteInfo == nullptr)
        {
            return;
        }

        auto position = simVectorToFloat(feature.position);
        const auto& sprite = *spriteInfo->animation->sprites[0];

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = featureDefinition.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();

        auto mvpMatrix = viewProjectionMatrix * modelMatrix;

        batch.sprites.push_back(SpriteRenderInfo{&sprite, mvpMatrix, spriteInfo->transparentAnimation});
    }

    void drawFeatureShadow(
        const UnitDatabase& unitDatabase,
        const MeshDatabase& meshDatabase,
        const MapFeature& feature,
        const Matrix4f& viewProjectionMatrix,
        SpriteBatch& batch)
    {
        const auto& featureDefinition = unitDatabase.getFeature(feature.featureName);
        const auto& featureMediaInfo = meshDatabase.getFeature(feature.featureName);

        auto spriteInfo = std::get_if<FeatureSpriteInfo>(&featureMediaInfo.renderInfo);
        if (spriteInfo == nullptr)
        {
            return;
        }

        if (!spriteInfo->shadowAnimation)
        {
            return;
        }
        auto position = simVectorToFloat(feature.position);
        const auto& sprite = *(*spriteInfo->shadowAnimation)->sprites[0];

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = featureDefinition.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();

        auto mvpMatrix = viewProjectionMatrix * modelMatrix;

        batch.sprites.push_back(SpriteRenderInfo{&sprite, mvpMatrix, spriteInfo->transparentShadow});
    }

    void drawNanoLine(const Vector3f& start, const Vector3f& end, ColoredMeshBatch& batch)
    {
        pushLine(batch.lines, start, end, Vector3f(0.0f, 1.0f, 0.0f));
    }

    void drawWakeParticle(const MeshDatabase& meshDatabase, GameTime currentTime, const Matrix4f& viewProjectionMatrix, const Particle& particle, ColoredMeshBatch& batch)
    {
        auto wakeRenderInfo = std::get_if<ParticleRenderTypeWake>(&particle.renderType);
        if (wakeRenderInfo == nullptr)
        {
            return;
        }

        if (!particle.isStarted(currentTime) || currentTime >= wakeRenderInfo->finishTime)
        {
            return;
        }

        const auto topLeft = particle.position + Vector3f(-1.0f, 0.0f, -1.0f);
        const auto topRight = particle.position + Vector3f(1.0f, 0.0f, -1.0f);
        const auto bottomLeft = particle.position + Vector3f(-1.0f, 0.0f, 1.0f);
        const auto bottomRight = particle.position + Vector3f(1.0f, 0.0f, 1.0f);

        Vector3f startColor(1.0f, 1.0f, 1.0f);
        Vector3f finishColor(0.4f, 0.5f, 1.0f);
        auto duration = wakeRenderInfo->finishTime - particle.startTime;
        auto timeElapsed = currentTime - particle.startTime;
        auto color = lerp(startColor, finishColor, static_cast<float>(timeElapsed.value) / static_cast<float>(duration.value));

        pushTriangle(batch.triangles, topLeft, bottomLeft, bottomRight, color);
        pushTriangle(batch.triangles, topLeft, bottomRight, topRight, color);
    }

    void drawSpriteParticle(const MeshDatabase& meshDatabase, GameTime currentTime, const Matrix4f& viewProjectionMatrix, const Particle& particle, SpriteBatch& batch)
    {
        auto spriteRenderInfo = std::get_if<ParticleRenderTypeSprite>(&particle.renderType);
        if (spriteRenderInfo == nullptr)
        {
            return;
        }

        auto spriteSeries = meshDatabase.getSpriteSeries(spriteRenderInfo->gafName, spriteRenderInfo->animName).value();

        if (!particle.isStarted(currentTime) || particle.isFinished(currentTime, spriteRenderInfo->finishTime, spriteRenderInfo->frameDuration, spriteSeries->sprites.size()))
        {
            return;
        }

        auto frameIndex = particle.getFrameIndex(currentTime, spriteRenderInfo->frameDuration, spriteSeries->sprites.size());
        const auto& sprite = *spriteSeries->sprites[frameIndex];

        Vector3f snappedPosition(
            std::round(particle.position.x),
            truncateToInterval(particle.position.y, 2.0f),
            std::round(particle.position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
        auto mvpMatrix = viewProjectionMatrix * modelMatrix;

        batch.sprites.push_back(SpriteRenderInfo{&sprite, mvpMatrix, spriteRenderInfo->translucent});
    }

    void updateParticles(const MeshDatabase& meshDatabase, GameTime currentTime, std::vector<Particle>& particles)
    {
        auto end = particles.end();
        for (auto it = particles.begin(); it != end;)
        {
            auto& particle = *it;
            auto isFinished = match(
                particle.renderType,
                [&](const ParticleRenderTypeSprite& s) {
                    const auto anim = meshDatabase.getSpriteSeries(s.gafName, s.animName).value();
                    return particle.isFinished(currentTime, s.finishTime, s.frameDuration, anim->sprites.size());
                },
                [&](const ParticleRenderTypeWake& w) {
                    return currentTime >= w.finishTime;
                });

            if (isFinished)
            {
                particle = std::move(*--end);
                continue;
            }

            particle.position += particle.velocity;

            ++it;
        }
        particles.erase(end, particles.end());
    }

    /**
     * Assumes input vector is normalised.
     */
    Matrix4f pointDirection(const Vector3f& direction)
    {
        auto right = direction.cross(Vector3f(0.0f, 1.0f, 0.0f)).normalizedOr(Vector3f(1.0f, 0.0f, 0.0f));
        auto realUp = right.cross(direction);
        return Matrix4f::rotationToAxes(right, realUp, direction);
    }

    Matrix4f rotationModeToMatrix(ProjectileRenderTypeModel::RotationMode r)
    {
        switch (r)
        {
            case ProjectileRenderTypeModel::RotationMode::HalfZ:
                return Matrix4f::rotationZ(0.0f, -1.0f); // 180 degrees
            case ProjectileRenderTypeModel::RotationMode::QuarterY:
                return Matrix4f::rotationY(1.0f, 0.0f); // 90 degrees
            case ProjectileRenderTypeModel::RotationMode::None:
                return Matrix4f::identity();
            default:
                throw std::logic_error("Unknown RotationMode");
        }
    }

    unsigned int getFrameIndex(GameTime currentTime, unsigned int numFrames)
    {
        return (currentTime.value / 2) % numFrames;
    }

    void drawProjectiles(
        const UnitDatabase& unitDatabase,
        const MeshDatabase& meshDatabase,
        const Matrix4f& viewProjectionMatrix,
        const VectorMap<Projectile, ProjectileIdTag>& projectiles,
        GameTime currentTime,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        ColoredMeshBatch& coloredMeshbatch,
        SpriteBatch& spriteBatch,
        UnitMeshBatch& unitMeshBatch)
    {
        for (const auto& e : projectiles)
        {
            const auto& projectile = e.second;
            auto position = lerp(simVectorToFloat(projectile.previousPosition), simVectorToFloat(projectile.position), frac);

            const auto& weaponMediaInfo = meshDatabase.getWeapon(projectile.weaponType);

            match(
                weaponMediaInfo.renderType,
                [&](const ProjectileRenderTypeLaser& l) {
                    Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

                    std::vector<GlColoredVertex> laserVertices;
                    auto backPosition = lerp(simVectorToFloat(projectile.getPreviousBackPosition(l.duration)), simVectorToFloat(projectile.getBackPosition(l.duration)), frac);

                    coloredMeshbatch.lines.emplace_back(position, l.color);
                    coloredMeshbatch.lines.emplace_back(backPosition, l.color);

                    coloredMeshbatch.lines.emplace_back(position + pixelOffset, l.color2);
                    coloredMeshbatch.lines.emplace_back(backPosition + pixelOffset, l.color2);
                },
                [&](const ProjectileRenderTypeModel& m) {
                    auto transform = Matrix4f::translation(position)
                        * pointDirection(simVectorToFloat(projectile.velocity).normalized())
                        * rotationModeToMatrix(m.rotationMode);
                    drawProjectileUnitMesh(&unitDatabase, meshDatabase, viewProjectionMatrix, m.objectName, transform, PlayerColorIndex(0), false, unitTextureAtlas, unitTeamTextureAtlases, unitMeshBatch);
                },
                [&](const ProjectileRenderTypeSprite& s) {
                    Vector3f snappedPosition(
                        std::round(position.x),
                        truncateToInterval(position.y, 2.0f),
                        std::round(position.z));
                    Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));
                    const auto spriteSeries = meshDatabase.getSpriteSeries(s.gaf, s.anim).value();
                    const auto& sprite = *spriteSeries->sprites[getFrameIndex(currentTime, spriteSeries->sprites.size())];
                    auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
                    auto mvpMatrix = viewProjectionMatrix * modelMatrix;
                    spriteBatch.sprites.push_back(SpriteRenderInfo{&sprite, mvpMatrix, false});
                },
                [&](const ProjectileRenderTypeFlamethrower&) {
                    Vector3f snappedPosition(
                        std::round(position.x),
                        truncateToInterval(position.y, 2.0f),
                        std::round(position.z));
                    Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));
                    const auto spriteSeries = meshDatabase.getSpriteSeries("FX", "flamestream").value();
                    auto timeSinceSpawn = currentTime - projectile.createdAt;
                    auto fullLifetime = projectile.dieOnFrame.value() - projectile.createdAt;
                    auto percentComplete = static_cast<float>(timeSinceSpawn.value) / static_cast<float>(fullLifetime.value);
                    auto frameIndex = static_cast<unsigned int>(percentComplete * spriteSeries->sprites.size());
                    assert(frameIndex < spriteSeries->sprites.size());
                    const auto& sprite = *spriteSeries->sprites[frameIndex];
                    auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
                    auto mvpMatrix = viewProjectionMatrix * modelMatrix;
                    spriteBatch.sprites.push_back(SpriteRenderInfo{&sprite, mvpMatrix, true});
                },
                [&](const ProjectileRenderTypeLightning& l) {
                    Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

                    std::vector<GlColoredVertex> laserVertices;
                    auto backPosition = lerp(simVectorToFloat(projectile.getPreviousBackPosition(l.duration)), simVectorToFloat(projectile.getBackPosition(l.duration)), frac);

                    coloredMeshbatch.lines.emplace_back(position, l.color);
                    coloredMeshbatch.lines.emplace_back(backPosition, l.color);
                },
                [&](const ProjectileRenderTypeMindgun&) {
                    // TODO: implement mindgun if anyone actually uses it
                },
                [&](const ProjectileRenderTypeNone&) {
                    // do nothing
                });
        }
    }

    void drawSelectionRect(const MeshDatabase& meshDatabase, const Matrix4f& viewProjectionMatrix, const UnitState& unit, const UnitDefinition& unitDefinition, float frac, ColoredMeshesBatch& batch)
    {
        auto selectionMesh = meshDatabase.getSelectionMesh(unitDefinition.objectName);

        auto position = lerp(simVectorToFloat(unit.previousPosition), simVectorToFloat(unit.position), frac);

        // try to ensure that the selection rectangle vertices
        // are aligned with the middle of pixels,
        // to prevent discontinuities in the drawn lines.
        Vector3f snappedPosition(
            snapToInterval(position.x, 1.0f) + 0.5f,
            snapToInterval(position.y, 2.0f),
            snapToInterval(position.z, 1.0f) + 0.5f);

        auto rotation = angleLerp(toRadians(unit.previousRotation).value, toRadians(unit.rotation).value, frac);
        auto matrix = Matrix4f::translation(snappedPosition) * Matrix4f::rotationY(rotation);
        auto mvpMatrix = viewProjectionMatrix * matrix;

        batch.meshes.push_back(ColoredMeshRenderInfo{selectionMesh.value().get(), mvpMatrix});
    }
}
