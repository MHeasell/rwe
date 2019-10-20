#include "RenderService.h"
#include <rwe/math/rwe_math.h>
#include <rwe/matrix_util.h>
#include <rwe/overloaded.h>

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

    RenderService::RenderService(
        GraphicsContext* graphics,
        ShaderService* shaders,
        const CabinetCamera& camera)
        : graphics(graphics),
          shaders(shaders),
          camera(camera)
    {
    }

    void
    RenderService::drawSelectionRect(const Unit& unit)
    {
        // try to ensure that the selection rectangle vertices
        // are aligned with the middle of pixels,
        // to prevent discontinuities in the drawn lines.
        Vector3f snappedPosition(
            snapToInterval(simScalarToFloat(unit.position.x), 1.0f) + 0.5f,
            snapToInterval(simScalarToFloat(unit.position.y), 2.0f),
            snapToInterval(simScalarToFloat(unit.position.z), 1.0f) + 0.5f);

        auto matrix = Matrix4f::translation(snappedPosition) * Matrix4f::rotationY(toRadians(unit.rotation).value);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * matrix);
        graphics->setUniformFloat(shader.alpha, 1.0f);
        graphics->drawLineLoop(unit.selectionMesh.visualMesh);
    }

    void RenderService::drawUnit(const Unit& unit, float seaLevel, float time)
    {
        if (unit.isBeingBuilt())
        {
            drawBuildingUnitMesh(unit.mesh, toFloatMatrix(unit.getTransform()), seaLevel, unit.getPreciseCompletePercent(), simScalarToFloat(unit.position.y), time);
        }
        else
        {
            drawUnitMesh(unit.mesh, toFloatMatrix(unit.getTransform()), seaLevel);
        }
    }

    void RenderService::drawUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, float seaLevel)
    {
        auto matrix = modelMatrix * toFloatMatrix(mesh.getTransform());

        if (mesh.visible)
        {
            drawShaderMesh(*mesh.mesh, matrix, seaLevel, mesh.shaded);
        }

        for (const auto& c : mesh.children)
        {
            drawUnitMesh(c, matrix, seaLevel);
        }
    }

    void RenderService::drawBuildingUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, float seaLevel, float percentComplete, float unitY, float time)
    {
        auto matrix = modelMatrix * toFloatMatrix(mesh.getTransform());

        if (mesh.visible)
        {
            auto mvpMatrix = camera.getViewProjectionMatrix() * matrix;

            // TODO: draw colored vertices

            {
                const auto& buildShader = shaders->unitBuild;
                graphics->bindShader(buildShader.handle.get());
                graphics->bindTexture(mesh.mesh->texture.get());
                graphics->setUniformMatrix(buildShader.mvpMatrix, mvpMatrix);
                graphics->setUniformMatrix(buildShader.modelMatrix, matrix);
                graphics->setUniformFloat(buildShader.unitY, unitY);
                graphics->setUniformFloat(buildShader.seaLevel, seaLevel);
                graphics->setUniformBool(buildShader.shade, mesh.shaded);
                graphics->setUniformFloat(buildShader.percentComplete, percentComplete);
                graphics->setUniformFloat(buildShader.time, time);
                graphics->drawTriangles(mesh.mesh->texturedVertices);
            }
        }

        for (const auto& c : mesh.children)
        {
            drawBuildingUnitMesh(c, matrix, seaLevel, percentComplete, unitY, time);
        }
    }

    void RenderService::drawOccupiedGrid(const MapTerrain& terrain, const OccupiedGrid& occupiedGrid)
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

        std::vector<Line3f> lines;

        std::vector<Triangle3f> tris;
        std::vector<Triangle3f> buildingTris;
        std::vector<Triangle3f> passableBuildingTris;

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

                lines.emplace_back(pos, rightPos);
                lines.emplace_back(pos, downPos);

                const auto& cell = occupiedGrid.get(x, y);
                if (std::visit(IsOccupiedVisitor(), cell.occupiedType))
                {
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);

                    tris.emplace_back(pos, downPos, downRightPos);
                    tris.emplace_back(pos, downRightPos, rightPos);
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

                    buildingTris.emplace_back(topLeftPos, bottomLeftPos, downRightPos);
                    buildingTris.emplace_back(topLeftPos, downRightPos, topRightPos);
                }

                if (cell.buildingCell && cell.buildingCell->passable)
                {
                    auto topLeftPos = pos + Vector3f(insetAmount, 0.0f, insetAmount);
                    auto topRightPos = rightPos + Vector3f(-insetAmount, 0.0f, insetAmount);
                    auto bottomLeftPos = downPos + Vector3f(insetAmount, 0.0f, -insetAmount);
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);
                    downRightPos += Vector3f(-insetAmount, 0.0f, -insetAmount);

                    passableBuildingTris.emplace_back(topLeftPos, bottomLeftPos, downRightPos);
                    passableBuildingTris.emplace_back(topLeftPos, downRightPos, topRightPos);
                }
            }
        }

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());
        graphics->setUniformFloat(shader.alpha, 1.0f);

        auto mesh = createTemporaryLinesMesh(lines);
        graphics->drawLines(mesh);

        auto triMesh = createTemporaryTriMesh(tris);
        graphics->drawTriangles(triMesh);

        auto buildingTriMesh = createTemporaryTriMesh(buildingTris, Vector3f(1.0f, 0.0f, 0.0f));
        graphics->drawTriangles(buildingTriMesh);

        auto passableBuildingTriMesh = createTemporaryTriMesh(passableBuildingTris, Vector3f(0.0f, 1.0f, 0.0f));
        graphics->drawTriangles(passableBuildingTriMesh);
    }

    void RenderService::drawMovementClassCollisionGrid(
        const MapTerrain& terrain,
        const Grid<char>& movementClassGrid)
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
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(floatToSimVector(Vector3f(right, 0.0f, bottom)));
        bottomRightCell.y += 7; // compensate for height
        bottomRightCell.x = std::clamp(bottomRightCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        bottomRightCell.y = std::clamp(bottomRightCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        assert(topLeftCell.x <= bottomRightCell.x);
        assert(topLeftCell.y <= bottomRightCell.y);

        std::vector<Line3f> lines;

        std::vector<Triangle3f> tris;

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

                lines.emplace_back(pos, rightPos);
                lines.emplace_back(pos, downPos);

                if (!movementClassGrid.get(x, y))
                {
                    auto downRightPos = simVectorToFloat(terrain.heightmapIndexToWorldCorner(x + 1, y + 1));
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);

                    tris.emplace_back(pos, downPos, downRightPos);
                    tris.emplace_back(pos, downRightPos, rightPos);
                }
            }
        }

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());
        graphics->setUniformFloat(shader.alpha, 1.0f);

        auto mesh = createTemporaryLinesMesh(lines);
        graphics->drawLines(mesh);

        auto triMesh = createTemporaryTriMesh(tris);
        graphics->drawTriangles(triMesh);
    }

    void
    RenderService::drawPathfindingVisualisation(const MapTerrain& terrain, const AStarPathInfo<Point, PathCost>& pathInfo)
    {
        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());
        graphics->setUniformFloat(shader.alpha, 1.0f);

        for (const auto& item : pathInfo.closedVertices)
        {
            if (!item.second.predecessor)
            {
                continue;
            }

            auto start = (*item.second.predecessor)->vertex;
            auto end = item.second.vertex;
            drawTerrainArrow(terrain, start, end, Color(255, 0, 0));
        }

        if (pathInfo.path.size() > 1)
        {
            for (auto it = pathInfo.path.begin() + 1; it != pathInfo.path.end(); ++it)
            {
                auto start = *(it - 1);
                auto end = *it;
                drawTerrainArrow(terrain, start, end, Color(0, 0, 255));
            }
        }
    }

    GlMesh RenderService::createTemporaryLinesMesh(const std::vector<Line3f>& lines)
    {
        return createTemporaryLinesMesh(lines, Color(255, 255, 255));
    }

    GlMesh RenderService::createTemporaryLinesMesh(const std::vector<Line3f>& lines, const Color& color)
    {
        std::vector<GlColoredVertex> buffer;
        buffer.reserve(lines.size() * 2); // 2 verts per line

        Vector3f floatColor(static_cast<float>(color.r) / 255.0f, static_cast<float>(color.g) / 255.0f, static_cast<float>(color.b) / 255.0f);

        for (const auto& l : lines)
        {
            buffer.emplace_back(l.start, floatColor);
            buffer.emplace_back(l.end, floatColor);
        }

        return graphics->createColoredMesh(buffer, GL_STREAM_DRAW);
    }

    GlMesh RenderService::createTemporaryTriMesh(const std::vector<Triangle3f>& tris)
    {
        return createTemporaryTriMesh(tris, Vector3f(1.0f, 1.0f, 1.0f));
    }

    GlMesh RenderService::createTemporaryTriMesh(const std::vector<Triangle3f>& tris, const Vector3f& color)
    {
        std::vector<GlColoredVertex> buffer;
        buffer.reserve(tris.size() * 3); // 3 verts per triangle

        for (const auto& l : tris)
        {
            buffer.emplace_back(l.a, color);
            buffer.emplace_back(l.b, color);
            buffer.emplace_back(l.c, color);
        }

        return graphics->createColoredMesh(buffer, GL_STREAM_DRAW);
    }

    void RenderService::drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        std::unordered_map<TextureIdentifier, std::vector<std::pair<unsigned int, unsigned int>>> batches;

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                batches[tileTexture.texture.get()].emplace_back(dx, dy);
            }
        }

        const auto& shader = shaders->basicTexture;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());

        for (const auto& batch : batches)
        {
            std::vector<GlTexturedVertex> vertices;

            for (const auto& p : batch.second)
            {
                auto dx = p.first;
                auto dy = p.second;

                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                auto tilePosition = simVectorToFloat(terrain.tileCoordinateToWorldCorner(x + dx, y + dy));

                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z), tileTexture.region.topLeft());
                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z + simScalarToFloat(MapTerrain::TileHeightInWorldUnits)), tileTexture.region.bottomLeft());
                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrain::TileWidthInWorldUnits), 0.0f, tilePosition.z + simScalarToFloat(MapTerrain::TileHeightInWorldUnits)), tileTexture.region.bottomRight());

                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrain::TileWidthInWorldUnits), 0.0f, tilePosition.z + simScalarToFloat(MapTerrain::TileHeightInWorldUnits)), tileTexture.region.bottomRight());
                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrain::TileWidthInWorldUnits), 0.0f, tilePosition.z), tileTexture.region.topRight());
                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z), tileTexture.region.topLeft());
            }

            auto mesh = graphics->createTexturedMesh(vertices, GL_STREAM_DRAW);

            graphics->bindTexture(batch.first);
            graphics->drawTriangles(mesh);
        }
    }

    void RenderService::drawMapTerrain(const MapTerrain& terrain)
    {
        Vector3f cameraExtents(camera.getWidth() / 2.0f, 0.0f, camera.getHeight() / 2.0f);
        auto topLeft = terrain.worldToTileCoordinate(floatToSimVector(camera.getPosition() - cameraExtents));
        auto bottomRight = terrain.worldToTileCoordinate(floatToSimVector(camera.getPosition() + cameraExtents));
        auto x1 = static_cast<unsigned int>(std::clamp<int>(topLeft.x, 0, terrain.getTiles().getWidth() - 1));
        auto y1 = static_cast<unsigned int>(std::clamp<int>(topLeft.y, 0, terrain.getTiles().getHeight() - 1));
        auto x2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.x, 0, terrain.getTiles().getWidth() - 1));
        auto y2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.y, 0, terrain.getTiles().getHeight() - 1));

        drawMapTerrain(terrain, x1, y1, (x2 + 1) - x1, (y2 + 1) - y1);
    }

    void RenderService::drawUnitShadow(const Unit& unit, float groundHeight)
    {
        auto shadowProjection = Matrix4f::translation(Vector3f(0.0f, groundHeight, 0.0f))
            * Matrix4f::scale(Vector3f(1.0f, 0.0f, 1.0f))
            * Matrix4f::shearXZ(0.25f, -0.25f)
            * Matrix4f::translation(Vector3f(0.0f, -groundHeight, 0.0f));

        auto matrix = Matrix4f::translation(simVectorToFloat(unit.position)) * Matrix4f::rotationY(toRadians(unit.rotation).value);

        drawUnitMesh(unit.mesh, shadowProjection * matrix, 0.0f);
    }

    CabinetCamera& RenderService::getCamera()
    {
        return camera;
    }

    const CabinetCamera& RenderService::getCamera() const
    {
        return camera;
    }

    void RenderService::fillScreen(float r, float g, float b, float a)
    {
        auto floatColor = Vector3f(r, g, b);

        // clang-format off
        std::vector<GlColoredVertex> vertices{
            {{-1.0f, -1.0f, 0.0f}, floatColor},
            {{ 1.0f, -1.0f, 0.0f}, floatColor},
            {{ 1.0f,  1.0f, 0.0f}, floatColor},

            {{ 1.0f,  1.0f, 0.0f}, floatColor},
            {{-1.0f,  1.0f, 0.0f}, floatColor},
            {{-1.0f, -1.0f, 0.0f}, floatColor},
        };
        // clang-format on

        auto mesh = graphics->createColoredMesh(vertices, GL_STREAM_DRAW);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, Matrix4f::identity());
        graphics->setUniformFloat(shader.alpha, a);
        graphics->drawTriangles(mesh);
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
        return (currentTime.value / 4) % numFrames;
    }

    void RenderService::drawProjectiles(const VectorMap<Projectile, ProjectileIdTag>& projectiles, float seaLevel, GameTime currentTime)
    {
        Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

        std::vector<GlColoredVertex> laserVertices;
        for (const auto& e : projectiles)
        {
            const auto& projectile = e.second;
            auto position = simVectorToFloat(projectile.position);

            match(
                projectile.renderType,
                [&](const ProjectileRenderTypeLaser& l) {
                    auto backPosition = simVectorToFloat(projectile.getBackPosition(l));

                    laserVertices.emplace_back(position, l.color);
                    laserVertices.emplace_back(backPosition, l.color);

                    laserVertices.emplace_back(position + pixelOffset, l.color2);
                    laserVertices.emplace_back(backPosition + pixelOffset, l.color2);
                },
                [&](const ProjectileRenderTypeModel& m) {
                    auto transform = Matrix4f::translation(position)
                        * pointDirection(simVectorToFloat(projectile.velocity).normalized())
                        * rotationModeToMatrix(m.rotationMode);
                    drawUnitMesh(*m.mesh, transform, seaLevel);
                },
                [&](const ProjectileRenderTypeSprite& s) {
                    Vector3f snappedPosition(
                        std::round(position.x),
                        truncateToInterval(position.y, 2.0f),
                        std::round(position.z));
                    Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));
                    const auto& shader = shaders->basicTexture;
                    graphics->bindShader(shader.handle.get());
                    const auto& sprite = *s.spriteSeries->sprites[getFrameIndex(currentTime, s.spriteSeries->sprites.size())];
                    auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
                    graphics->bindTexture(sprite.texture.get());
                    graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
                    graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, 1.0f);
                    graphics->drawTriangles(*sprite.mesh);
                },
                [&](const auto&) {
                    // TODO: implement other render types
                });
        }

        auto mesh = graphics->createColoredMesh(laserVertices, GL_STREAM_DRAW);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());
        graphics->setUniformFloat(shader.alpha, 1.0f);

        graphics->drawLines(mesh);
    }

    void RenderService::drawExplosions(GameTime currentTime, const std::vector<Explosion>& explosions)
    {
        graphics->bindShader(shaders->basicTexture.handle.get());

        for (const auto& exp : explosions)
        {
            if (!exp.isStarted(currentTime) || exp.isFinished(currentTime))
            {
                continue;
            }

            auto position = simVectorToFloat(exp.position);
            auto frameIndex = exp.getFrameIndex(currentTime);
            const auto& sprite = *exp.animation->sprites[frameIndex];

            float alpha = 1.0f;

            Vector3f snappedPosition(
                std::round(position.x),
                truncateToInterval(position.y, 2.0f),
                std::round(position.z));

            // Convert to a model position that makes sense in the game world.
            // For standing (blocking) features we stretch y-dimension values by 2x
            // to correct for TA camera distortion.
            Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));

            auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();

            const auto& shader = shaders->basicTexture;
            graphics->bindTexture(sprite.texture.get());
            graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
            graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, alpha);
            graphics->drawTriangles(*sprite.mesh);
        }
    }

    void RenderService::drawShaderMesh(const ShaderMesh& mesh, const Matrix4f& matrix, float seaLevel, bool shaded)
    {
        auto mvpMatrix = camera.getViewProjectionMatrix() * matrix;

        {
            const auto& textureShader = shaders->unitTexture;
            graphics->bindShader(textureShader.handle.get());
            graphics->bindTexture(mesh.texture.get());
            graphics->setUniformMatrix(textureShader.mvpMatrix, mvpMatrix);
            graphics->setUniformMatrix(textureShader.modelMatrix, matrix);
            graphics->setUniformFloat(textureShader.seaLevel, seaLevel);
            graphics->setUniformBool(textureShader.shade, shaded);
            graphics->drawTriangles(mesh.texturedVertices);
        }
    }

    void RenderService::drawFeatureShadowInternal(const MapFeature& feature)
    {
        if (!feature.shadowAnimation)
        {
            return;
        }

        auto position = simVectorToFloat(feature.position);
        const auto& sprite = *(*feature.shadowAnimation)->sprites[0];

        float alpha = feature.transparentShadow ? 0.5f : 1.0f;

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = feature.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();

        const auto& shader = shaders->basicTexture;
        graphics->bindTexture(sprite.texture.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
        graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, alpha);
        graphics->drawTriangles(*sprite.mesh);
    }

    void RenderService::drawFeatureInternal(const MapFeature& feature)
    {
        auto position = simVectorToFloat(feature.position);
        const auto& sprite = *feature.animation->sprites[0];

        float alpha = feature.transparentAnimation ? 0.5f : 1.0f;

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = feature.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();

        const auto& shader = shaders->basicTexture;
        graphics->bindTexture(sprite.texture.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
        graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, alpha);
        graphics->drawTriangles(*sprite.mesh);
    }

    void
    RenderService::drawTerrainArrow(const MapTerrain& terrain, const Point& start, const Point& end, const Color& color)
    {
        std::vector<Line3f> lines;

        auto worldStart = terrain.heightmapIndexToWorldCenter(start);
        worldStart.y = terrain.getHeightAt(worldStart.x, worldStart.z);
        auto worldStartF = simVectorToFloat(worldStart);

        auto worldEnd = terrain.heightmapIndexToWorldCenter(end);
        worldEnd.y = terrain.getHeightAt(worldEnd.x, worldEnd.z);
        auto worldEndF = simVectorToFloat(worldEnd);

        auto armTemplate = (worldStartF - worldEndF).normalized() * 6.0f;
        auto arm1 = Matrix4f::translation(worldEndF) * Matrix4f::rotationY(Pif / 6.0f) * armTemplate;
        auto arm2 = Matrix4f::translation(worldEndF) * Matrix4f::rotationY(-Pif / 6.0f) * armTemplate;

        lines.emplace_back(worldStartF, worldEndF);
        lines.emplace_back(worldEndF, arm1);
        lines.emplace_back(worldEndF, arm2);

        graphics->drawLines(createTemporaryLinesMesh(lines, color));
    }
}
