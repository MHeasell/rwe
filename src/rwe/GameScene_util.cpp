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

    Matrix4x<SimScalar> getPieceTransform(const std::string& pieceName, const UnitModelDefinition& modelDefinition, const std::vector<UnitMesh>& pieces)
    {
        assert(modelDefinition.pieces.size() == pieces.size());

        std::optional<std::string> parentPiece = pieceName;
        auto matrix = Matrix4x<SimScalar>::identity();

        do
        {
            auto pieceIndexIt = modelDefinition.pieceIndicesByName.find(toUpper(*parentPiece));
            if (pieceIndexIt == modelDefinition.pieceIndicesByName.end())
            {
                throw std::runtime_error("missing piece definition: " + *parentPiece);
            }

            const auto& pieceDef = modelDefinition.pieces.at(pieceIndexIt->second);

            parentPiece = pieceDef.parent;

            auto pieceStateIt = pieces.begin() + pieceIndexIt->second;

            auto position = pieceDef.origin + pieceStateIt->offset;
            auto rotationX = pieceStateIt->rotationX;
            auto rotationY = pieceStateIt->rotationY;
            auto rotationZ = pieceStateIt->rotationZ;
            matrix = Matrix4x<SimScalar>::translation(position)
                * Matrix4x<SimScalar>::rotationZXY(
                    sin(rotationX),
                    cos(rotationX),
                    sin(rotationY),
                    cos(rotationY),
                    sin(rotationZ),
                    cos(rotationZ))
                * matrix;
        } while (parentPiece);

        return matrix;
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

    void drawOccupiedGrid(const CabinetCamera& camera, const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, ColoredMeshBatch& batch)
    {
        auto halfWidth = camera.getWidth() / 2.0f;
        auto halfHeight = camera.getHeight() / 2.0f;
        auto left = camera.getPosition().x - halfWidth;
        auto top = camera.getPosition().z - halfHeight;
        auto right = camera.getPosition().x + halfWidth;
        auto bottom = camera.getPosition().z + halfHeight;

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
        const CabinetCamera& camera,
        ColoredMeshBatch& batch)
    {
        auto halfWidth = camera.getWidth() / 2.0f;
        auto halfHeight = camera.getHeight() / 2.0f;
        auto left = camera.getPosition().x - halfWidth;
        auto top = camera.getPosition().z - halfHeight;
        auto right = camera.getPosition().x + halfWidth;
        auto bottom = camera.getPosition().z + halfHeight;

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
        const CabinetCamera& camera,
        const ShaderMesh& mesh,
        const Matrix4f& matrix,
        bool shaded,
        PlayerColorIndex playerColorIndex,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        std::vector<UnitTextureMeshRenderInfo>& batch)
    {
        auto mvpMatrix = camera.getViewProjectionMatrix() * matrix;

        batch.push_back(UnitTextureMeshRenderInfo{&mesh.vertices, matrix, mvpMatrix, shaded, unitTextureAtlas});
        batch.push_back(UnitTextureMeshRenderInfo{&mesh.teamVertices, matrix, mvpMatrix, shaded, unitTeamTextureAtlases.at(playerColorIndex.value).get()});
    }

    void drawUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const CabinetCamera& camera,
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

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value();
            drawShaderMesh(camera, resolvedMesh, matrix, mesh.shaded, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawBuildingShaderMesh(
        const CabinetCamera& camera,
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
        auto mvpMatrix = camera.getViewProjectionMatrix() * matrix;
        batch.push_back(UnitBuildingMeshRenderInfo{&mesh.vertices, matrix, mvpMatrix, shaded, unitTextureAtlas, percentComplete, unitY});
        batch.push_back(UnitBuildingMeshRenderInfo{&mesh.teamVertices, matrix, mvpMatrix, shaded, unitTeamTextureAtlases.at(playerColorIndex.value).get(), percentComplete, unitY});
    }

    void drawBuildingUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const CabinetCamera& camera,
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

            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value();
            drawBuildingShaderMesh(camera, resolvedMesh, matrix, mesh.shaded, percentComplete, unitY, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.buildingMeshes);
        }
    }

    void drawProjectileUnitMesh(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const CabinetCamera& camera,
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
            const auto& resolvedMesh = *meshDatabase.getUnitPieceMesh(objectName, pieceDef.name).value();
            drawShaderMesh(camera, resolvedMesh, matrix, shaded, playerColorIndex, unitTextureAtlas, unitTeamTextureAtlases, batch.meshes);
        }
    }

    void drawUnit(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const CabinetCamera& camera,
        const Unit& unit,
        PlayerColorIndex playerColorIndex,
        float frac,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {
        auto position = lerp(simVectorToFloat(unit.previousPosition), simVectorToFloat(unit.position), frac);
        auto rotation = angleLerp(toRadians(unit.previousRotation).value, toRadians(unit.rotation).value, frac);
        auto transform = Matrix4f::translation(position) * Matrix4f::rotationY(rotation);
        if (unit.isBeingBuilt())
        {
            drawBuildingUnitMesh(unitDatabase, meshDatabase, camera, unit.objectName, unit.pieces, transform, unit.getPreciseCompletePercent(), position.y, playerColorIndex, frac, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
        else
        {
            drawUnitMesh(unitDatabase, meshDatabase, camera, unit.objectName, unit.pieces, transform, playerColorIndex, frac, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
    }

    void drawMeshFeature(
        const UnitDatabase* unitDatabase,
        const MeshDatabase& meshDatabase,
        const CabinetCamera& camera,
        const MapFeature& feature,
        TextureIdentifier unitTextureAtlas,
        std::vector<SharedTextureHandle>& unitTeamTextureAtlases,
        UnitMeshBatch& batch)
    {

        if (auto objectInfo = std::get_if<FeatureObjectInfo>(&feature.renderInfo); objectInfo != nullptr)
        {
            auto matrix = Matrix4f::translation(simVectorToFloat(feature.position)) * Matrix4f::rotationY(0.0f, -1.0f);
            drawProjectileUnitMesh(unitDatabase, meshDatabase, camera, objectInfo->objectName, matrix, PlayerColorIndex(0), true, unitTextureAtlas, unitTeamTextureAtlases, batch);
        }
    }

    void updateExplosions(const MeshDatabase& meshDatabase, GameTime currentTime, std::vector<Explosion>& explosions)
    {
        auto end = explosions.end();
        for (auto it = explosions.begin(); it != end;)
        {
            auto& exp = *it;
            const auto anim = meshDatabase.getSpriteSeries(exp.explosionGaf, exp.explosionAnim).value();
            if (exp.isFinished(currentTime, anim->sprites.size()))
            {
                exp = std::move(*--end);
                continue;
            }

            if (exp.floats)
            {
                // TODO: drift with the wind
                exp.position.y += 0.5f;
            }

            ++it;
        }
        explosions.erase(end, explosions.end());
    }
}
