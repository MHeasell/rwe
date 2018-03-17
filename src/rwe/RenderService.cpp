#include "RenderService.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    class IsOccupiedVisitor : public boost::static_visitor<bool>
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
            snapToInterval(unit.position.x, 1.0f) + 0.5f,
            snapToInterval(unit.position.y, 2.0f),
            snapToInterval(unit.position.z, 1.0f) + 0.5f);

        auto matrix = Matrix4f::translation(snappedPosition) * Matrix4f::rotationY(unit.rotation);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * matrix);
        graphics->setUniformFloat(shader.alpha, 1.0f);
        graphics->drawLineLoop(unit.selectionMesh.visualMesh);
    }

    void RenderService::drawUnit(const Unit& unit, float seaLevel)
    {
        drawUnitMesh(unit.mesh, unit.getTransform(), seaLevel);
    }

    void RenderService::drawUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, float seaLevel)
    {
        auto matrix = modelMatrix * mesh.getTransform();

        if (mesh.visible)
        {
            auto mvpMatrix = camera.getViewProjectionMatrix() * matrix;

            {
                const auto& colorShader = shaders->unitColor;
                graphics->bindShader(colorShader.handle.get());
                graphics->setUniformMatrix(colorShader.mvpMatrix, mvpMatrix);
                graphics->setUniformMatrix(colorShader.modelMatrix, matrix);
                graphics->setUniformFloat(colorShader.seaLevel, seaLevel);
                graphics->drawTriangles(mesh.mesh->coloredVertices);
            }

            {
                const auto& textureShader = shaders->unitTexture;
                graphics->bindShader(textureShader.handle.get());
                graphics->bindTexture(mesh.mesh->texture.get());
                graphics->setUniformMatrix(textureShader.mvpMatrix, mvpMatrix);
                graphics->setUniformMatrix(textureShader.modelMatrix, matrix);
                graphics->setUniformFloat(textureShader.seaLevel, seaLevel);
                graphics->drawTriangles(mesh.mesh->texturedVertices);
            }
        }

        for (const auto& c : mesh.children)
        {
            drawUnitMesh(c, matrix, seaLevel);
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

        auto topLeftCell = terrain.worldToHeightmapCoordinate(Vector3f(left, 0.0f, top));
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(Vector3f(right, 0.0f, bottom));
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
                auto pos = terrain.heightmapIndexToWorldCorner(x, y);
                pos.y = terrain.getHeightMap().get(x, y);

                auto rightPos = terrain.heightmapIndexToWorldCorner(x + 1, y);
                rightPos.y = terrain.getHeightMap().get(x + 1, y);

                auto downPos = terrain.heightmapIndexToWorldCorner(x, y + 1);
                downPos.y = terrain.getHeightMap().get(x, y + 1);

                lines.emplace_back(pos, rightPos);
                lines.emplace_back(pos, downPos);

                if (boost::apply_visitor(IsOccupiedVisitor(), occupiedGrid.grid.get(x, y)))
                {
                    auto downRightPos = terrain.heightmapIndexToWorldCorner(x + 1, y + 1);
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

        auto topLeftCell = terrain.worldToHeightmapCoordinate(Vector3f(left, 0.0f, top));
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(Vector3f(right, 0.0f, bottom));
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
                auto pos = terrain.heightmapIndexToWorldCorner(x, y);
                pos.y = terrain.getHeightMap().get(x, y);

                auto rightPos = terrain.heightmapIndexToWorldCorner(x + 1, y);
                rightPos.y = terrain.getHeightMap().get(x + 1, y);

                auto downPos = terrain.heightmapIndexToWorldCorner(x, y + 1);
                downPos.y = terrain.getHeightMap().get(x, y + 1);

                lines.emplace_back(pos, rightPos);
                lines.emplace_back(pos, downPos);

                if (!movementClassGrid.get(x, y))
                {
                    auto downRightPos = terrain.heightmapIndexToWorldCorner(x + 1, y + 1);
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
        std::vector<GlColoredVertex> buffer;
        buffer.reserve(tris.size() * 3); // 3 verts per triangle

        Vector3f white(1.0f, 1.0f, 1.0f);

        for (const auto& l : tris)
        {
            buffer.emplace_back(l.a, white);
            buffer.emplace_back(l.b, white);
            buffer.emplace_back(l.c, white);
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
                auto tilePosition = terrain.tileCoordinateToWorldCorner(x + dx, y + dy);

                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z), tileTexture.region.topLeft());
                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits), tileTexture.region.bottomLeft());
                vertices.emplace_back(Vector3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits), tileTexture.region.bottomRight());

                vertices.emplace_back(Vector3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z + MapTerrain::TileHeightInWorldUnits), tileTexture.region.bottomRight());
                vertices.emplace_back(Vector3f(tilePosition.x + MapTerrain::TileWidthInWorldUnits, 0.0f, tilePosition.z), tileTexture.region.topRight());
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
        auto topLeft = terrain.worldToTileCoordinate(camera.getPosition() - cameraExtents);
        auto bottomRight = terrain.worldToTileCoordinate(camera.getPosition() + cameraExtents);
        auto x1 = static_cast<unsigned int>(std::clamp<int>(topLeft.x, 0, terrain.getTiles().getWidth() - 1));
        auto y1 = static_cast<unsigned int>(std::clamp<int>(topLeft.y, 0, terrain.getTiles().getHeight() - 1));
        auto x2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.x, 0, terrain.getTiles().getWidth() - 1));
        auto y2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.y, 0, terrain.getTiles().getHeight() - 1));

        drawMapTerrain(terrain, x1, y1, (x2 + 1) - x1, (y2 + 1) - y1);
    }

    void RenderService::drawFlatFeatures(const std::vector<MapFeature>& features)
    {
        drawFlatFeaturesInternal(features.begin(), features.end());
    }

    void RenderService::drawFlatFeatureShadows(const std::vector<MapFeature>& features)
    {
        drawFlatFeatureShadowsInternal(features.begin(), features.end());
    }

    void RenderService::drawStandingFeatures(const std::vector<MapFeature>& features)
    {
        drawStandingFeaturesInternal(features.begin(), features.end());
    }

    void RenderService::drawStandingFeatureShadows(const std::vector<MapFeature>& features)
    {
        drawStandingFeatureShadowsInternal(features.begin(), features.end());
    }

    void RenderService::drawUnitShadow(const Unit& unit, float groundHeight)
    {
        auto shadowProjection = Matrix4f::translation(Vector3f(0.0f, groundHeight, 0.0f))
            * Matrix4f::scale(Vector3f(1.0f, 0.0f, 1.0f))
            * Matrix4f::shearXZ(0.25f, -0.25f)
            * Matrix4f::translation(Vector3f(0.0f, -groundHeight, 0.0f));

        auto matrix = Matrix4f::translation(unit.position) * Matrix4f::rotationY(unit.rotation);

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

    void RenderService::drawUnitShadows(const MapTerrain& terrain, const std::vector<Unit>& units)
    {
        graphics->enableStencilBuffer();
        graphics->clearStencilBuffer();
        graphics->useStencilBufferForWrites();
        graphics->disableColorBuffer();

        for (const auto& unit : units)
        {
            auto groundHeight = terrain.getHeightAt(unit.position.x, unit.position.z);
            drawUnitShadow(unit, groundHeight);
        }

        graphics->useStencilBufferAsMask();
        graphics->enableColorBuffer();

        fillScreen(0.0f, 0.0f, 0.0f, 0.5f);

        graphics->enableColorBuffer();
        graphics->disableStencilBuffer();
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

    void RenderService::drawLasers(const std::vector<std::optional<LaserProjectile>>& lasers)
    {
        Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

        std::vector<GlColoredVertex> vertices;
        for (const auto& laser : lasers)
        {
            if (!laser)
            {
                continue;
            }

            auto backPosition = laser->getBackPosition();

            vertices.emplace_back(laser->position, laser->color);
            vertices.emplace_back(backPosition, laser->color);

            vertices.emplace_back(laser->position + pixelOffset, laser->color2);
            vertices.emplace_back(backPosition + pixelOffset, laser->color2);
        }

        auto mesh = graphics->createColoredMesh(vertices, GL_STREAM_DRAW);

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix());
        graphics->setUniformFloat(shader.alpha, 1.0f);

        graphics->drawLines(mesh);
    }

    void RenderService::drawExplosions(GameTime currentTime, const std::vector<std::optional<Explosion>>& explosions)
    {
        graphics->bindShader(shaders->basicTexture.handle.get());

        for (const auto& exp : explosions)
        {
            if (!exp)
            {
                continue;
            }

            if (!exp->isStarted(currentTime) || exp->isFinished(currentTime))
            {
                continue;
            }

            const auto& position = exp->position;
            auto frameIndex = exp->getFrameIndex(currentTime);
            const auto& sprite = *exp->animation->sprites[frameIndex];

            float alpha = 1.0f;

            Vector3f snappedPosition(
                std::round(position.x),
                truncateToInterval(position.y, 2.0f),
                std::round(position.z));

            // Convert to a model position that makes sense in the game world.
            // For standing (blocking) features we stretch y-dimension values by 2x
            // to correct for TA camera distortion.
            Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));

            auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix;

            const auto& shader = shaders->basicTexture;
            graphics->bindTexture(sprite.mesh.texture.get());
            graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
            graphics->setUniformFloat(shader.alpha, alpha);
            graphics->drawTriangles(sprite.mesh.mesh);
        }
    }

    void RenderService::drawFeatureShadowInternal(const MapFeature& feature)
    {
        if (!feature.shadowAnimation)
        {
            return;
        }

        const auto& position = feature.position;
        const auto& sprite = *(*feature.shadowAnimation)->sprites[0];

        float alpha = feature.transparentShadow ? 0.5f : 1.0f;

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a world-space flat position.
        auto modelMatrix = Matrix4f::translation(snappedPosition)
            * Matrix4f::rotationX(-Pif / 2.0f)
            * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        const auto& shader = shaders->basicTexture;
        graphics->bindTexture(sprite.mesh.texture.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
        graphics->setUniformFloat(shader.alpha, alpha);
        graphics->drawTriangles(sprite.mesh.mesh);
    }

    void RenderService::drawFeatureInternal(const MapFeature& feature)
    {
        const auto& position = feature.position;
        const auto& sprite = *feature.animation->sprites[0];

        float alpha = feature.transparentAnimation ? 0.5f : 1.0f;

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        Matrix4f conversionMatrix = feature.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix;

        const auto& shader = shaders->basicTexture;
        graphics->bindTexture(sprite.mesh.texture.get());
        graphics->setUniformMatrix(shader.mvpMatrix, camera.getViewProjectionMatrix() * modelMatrix);
        graphics->setUniformFloat(shader.alpha, alpha);
        graphics->drawTriangles(sprite.mesh.mesh);
    }

    void
    RenderService::drawTerrainArrow(const MapTerrain& terrain, const Point& start, const Point& end, const Color& color)
    {
        std::vector<Line3f> lines;

        auto worldStart = terrain.heightmapIndexToWorldCenter(start);
        worldStart.y = terrain.getHeightAt(worldStart.x, worldStart.z);

        auto worldEnd = terrain.heightmapIndexToWorldCenter(end);
        worldEnd.y = terrain.getHeightAt(worldEnd.x, worldEnd.z);

        auto armTemplate = (worldStart - worldEnd).normalized() * 6.0f;
        auto arm1 = Matrix4f::translation(worldEnd) * Matrix4f::rotationY(Pif / 6.0f) * armTemplate;
        auto arm2 = Matrix4f::translation(worldEnd) * Matrix4f::rotationY(-Pif / 6.0f) * armTemplate;

        lines.emplace_back(worldStart, worldEnd);
        lines.emplace_back(worldEnd, arm1);
        lines.emplace_back(worldEnd, arm2);

        graphics->drawLines(createTemporaryLinesMesh(lines, color));
    }
}
