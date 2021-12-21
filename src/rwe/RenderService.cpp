#include "RenderService.h"
#include <rwe/Index.h>
#include <rwe/match.h>
#include <rwe/math/rwe_math.h>
#include <rwe/matrix_util.h>

namespace rwe
{
    Matrix4f getPieceTransform(const std::string& pieceName, const UnitModelDefinition& modelDefinition)
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

    RenderService::RenderService(
        GraphicsContext* graphics,
        ShaderService* shaders,
        const MeshDatabase* meshDatabase,
        UnitDatabase* unitDatabase,
        const Matrix4f* viewProjectionMatrix,
        const SharedTextureHandle* unitTextureAtlas,
        const std::vector<SharedTextureHandle>* unitTeamTextureAtlases)
        : graphics(graphics),
          shaders(shaders),
          meshDatabase(meshDatabase),
          unitDatabase(unitDatabase),
          viewProjectionMatrix(viewProjectionMatrix),
          unitTextureAtlas(unitTextureAtlas),
          unitTeamTextureAtlases(unitTeamTextureAtlases)
    {
    }

    void
    RenderService::drawSelectionRect(const Unit& unit, float frac)
    {
        auto selectionMesh = meshDatabase->getSelectionMesh(unit.objectName);

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

        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, (*viewProjectionMatrix) * matrix);
        graphics->setUniformFloat(shader.alpha, 1.0f);
        graphics->drawLineLoop(*selectionMesh.value());
    }

    void RenderService::drawProjectileUnitMesh(const std::string& objectName, const Matrix4f& modelMatrix, float seaLevel, PlayerColorIndex playerColorIndex, bool shaded)
    {
        auto modelDefinition = unitDatabase->getUnitModelDefinition(objectName);
        if (!modelDefinition)
        {
            throw std::runtime_error("missing model definition: " + objectName);
        }

        for (const auto& pieceDef : modelDefinition->get().pieces)
        {
            auto matrix = modelMatrix * getPieceTransform(pieceDef.name, modelDefinition->get());
            const auto& resolvedMesh = *meshDatabase->getUnitPieceMesh(objectName, pieceDef.name).value();
            drawShaderMesh(resolvedMesh, matrix, seaLevel, shaded, playerColorIndex);
        }
    }

    void RenderService::drawMapTerrain(const MapTerrainGraphics& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    {
        std::unordered_map<TextureArrayIdentifier, std::vector<std::pair<unsigned int, unsigned int>>> batches;

        for (unsigned int dy = 0; dy < height; ++dy)
        {
            for (unsigned int dx = 0; dx < width; ++dx)
            {
                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                const auto& tileTexture = terrain.getTileTexture(tileIndex);

                batches[tileTexture.texture.get()].emplace_back(dx, dy);
            }
        }

        const auto& shader = shaders->mapTerrain;
        graphics->bindShader(shader.handle.get());
        graphics->setUniformMatrix(shader.mvpMatrix, *viewProjectionMatrix);

        for (const auto& batch : batches)
        {
            std::vector<GlTextureArrayVertex> vertices;

            for (const auto& p : batch.second)
            {
                auto dx = p.first;
                auto dy = p.second;

                auto tileIndex = terrain.getTiles().get(x + dx, y + dy);
                auto tilePosition = simVectorToFloat(terrain.tileCoordinateToWorldCorner(x + dx, y + dy));

                const auto& tileTextureArray = terrain.getTileTexture(tileIndex);
                auto layerIndex = static_cast<float>(tileTextureArray.index);

                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z), Vector3f(0.0f, 0.0f, layerIndex));
                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z + simScalarToFloat(MapTerrainGraphics::TileHeightInWorldUnits)), Vector3f(0.0f, 1.0f, layerIndex));
                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrainGraphics::TileWidthInWorldUnits), 0.0f, tilePosition.z + simScalarToFloat(MapTerrainGraphics::TileHeightInWorldUnits)), Vector3f(1.0f, 1.0f, layerIndex));

                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrainGraphics::TileWidthInWorldUnits), 0.0f, tilePosition.z + simScalarToFloat(MapTerrainGraphics::TileHeightInWorldUnits)), Vector3f(1.0f, 1.0f, layerIndex));
                vertices.emplace_back(Vector3f(tilePosition.x + simScalarToFloat(MapTerrainGraphics::TileWidthInWorldUnits), 0.0f, tilePosition.z), Vector3f(1.0f, 0.0f, layerIndex));
                vertices.emplace_back(Vector3f(tilePosition.x, 0.0f, tilePosition.z), Vector3f(0.0f, 0.0f, layerIndex));
            }

            auto mesh = graphics->createTextureArrayMesh(vertices, GL_STREAM_DRAW);

            graphics->bindTextureArray(batch.first);
            graphics->drawTriangles(mesh);
        }
    }

    void RenderService::drawMapTerrain(const MapTerrainGraphics& terrain, const Vector3f& cameraPosition, float viewportWidth, float viewportHeight)
    {
        Vector3f cameraExtents(viewportWidth / 2.0f, 0.0f, viewportHeight / 2.0f);
        auto topLeft = terrain.worldToTileCoordinate(floatToSimVector(cameraPosition - cameraExtents));
        auto bottomRight = terrain.worldToTileCoordinate(floatToSimVector(cameraPosition + cameraExtents));
        auto x1 = static_cast<unsigned int>(std::clamp<int>(topLeft.x, 0, terrain.getTiles().getWidth() - 1));
        auto y1 = static_cast<unsigned int>(std::clamp<int>(topLeft.y, 0, terrain.getTiles().getHeight() - 1));
        auto x2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.x, 0, terrain.getTiles().getWidth() - 1));
        auto y2 = static_cast<unsigned int>(std::clamp<int>(bottomRight.y, 0, terrain.getTiles().getHeight() - 1));

        drawMapTerrain(terrain, x1, y1, (x2 + 1) - x1, (y2 + 1) - y1);
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
        return (currentTime.value / 2) % numFrames;
    }

    void RenderService::drawProjectiles(const VectorMap<Projectile, ProjectileIdTag>& projectiles, float seaLevel, GameTime currentTime, float frac)
    {
        for (const auto& e : projectiles)
        {
            const auto& projectile = e.second;
            auto position = lerp(simVectorToFloat(projectile.previousPosition), simVectorToFloat(projectile.position), frac);

            const auto& weaponMediaInfo = unitDatabase->getWeapon(projectile.weaponType);

            match(
                weaponMediaInfo.renderType,
                [&](const ProjectileRenderTypeLaser& l) {
                    Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

                    std::vector<GlColoredVertex> laserVertices;
                    auto backPosition = lerp(simVectorToFloat(projectile.getPreviousBackPosition(l.duration)), simVectorToFloat(projectile.getBackPosition(l.duration)), frac);

                    laserVertices.emplace_back(position, l.color);
                    laserVertices.emplace_back(backPosition, l.color);

                    laserVertices.emplace_back(position + pixelOffset, l.color2);
                    laserVertices.emplace_back(backPosition + pixelOffset, l.color2);

                    auto mesh = graphics->createColoredMesh(laserVertices, GL_STREAM_DRAW);

                    const auto& shader = shaders->basicColor;
                    graphics->bindShader(shader.handle.get());
                    graphics->setUniformMatrix(shader.mvpMatrix, *viewProjectionMatrix);
                    graphics->setUniformFloat(shader.alpha, 1.0f);

                    graphics->drawLines(mesh);
                },
                [&](const ProjectileRenderTypeModel& m) {
                    auto transform = Matrix4f::translation(position)
                        * pointDirection(simVectorToFloat(projectile.velocity).normalized())
                        * rotationModeToMatrix(m.rotationMode);
                    drawProjectileUnitMesh(m.objectName, transform, seaLevel, PlayerColorIndex(0), false);
                },
                [&](const ProjectileRenderTypeSprite& s) {
                    Vector3f snappedPosition(
                        std::round(position.x),
                        truncateToInterval(position.y, 2.0f),
                        std::round(position.z));
                    Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));
                    const auto& shader = shaders->basicTexture;
                    graphics->bindShader(shader.handle.get());
                    const auto spriteSeries = meshDatabase->getSpriteSeries(s.gaf, s.anim).value();
                    const auto& sprite = *spriteSeries->sprites[getFrameIndex(currentTime, spriteSeries->sprites.size())];
                    auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
                    graphics->bindTexture(sprite.texture.get());
                    graphics->setUniformMatrix(shader.mvpMatrix, (*viewProjectionMatrix) * modelMatrix);
                    graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, 1.0f);
                    graphics->drawTriangles(*sprite.mesh);
                },
                [&](const ProjectileRenderTypeFlamethrower&) {
                    Vector3f snappedPosition(
                        std::round(position.x),
                        truncateToInterval(position.y, 2.0f),
                        std::round(position.z));
                    Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));
                    const auto& shader = shaders->basicTexture;
                    graphics->bindShader(shader.handle.get());
                    const auto spriteSeries = meshDatabase->getSpriteSeries("FX", "flamestream").value();
                    auto timeSinceSpawn = currentTime - projectile.createdAt;
                    auto fullLifetime = projectile.dieOnFrame.value() - projectile.createdAt;
                    auto percentComplete = static_cast<float>(timeSinceSpawn.value) / static_cast<float>(fullLifetime.value);
                    auto frameIndex = static_cast<unsigned int>(percentComplete * spriteSeries->sprites.size());
                    assert(frameIndex < spriteSeries->sprites.size());
                    const auto& sprite = *spriteSeries->sprites[frameIndex];
                    auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
                    graphics->bindTexture(sprite.texture.get());
                    graphics->setUniformMatrix(shader.mvpMatrix, (*viewProjectionMatrix) * modelMatrix);
                    graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, 0.5f);
                    graphics->drawTriangles(*sprite.mesh);
                },
                [&](const ProjectileRenderTypeLightning& l) {
                    Vector3f pixelOffset(0.0f, 0.0f, -1.0f);

                    std::vector<GlColoredVertex> laserVertices;
                    auto backPosition = lerp(simVectorToFloat(projectile.getPreviousBackPosition(l.duration)), simVectorToFloat(projectile.getBackPosition(l.duration)), frac);

                    laserVertices.emplace_back(position, l.color);
                    laserVertices.emplace_back(backPosition, l.color);

                    auto mesh = graphics->createColoredMesh(laserVertices, GL_STREAM_DRAW);

                    const auto& shader = shaders->basicColor;
                    graphics->bindShader(shader.handle.get());
                    graphics->setUniformMatrix(shader.mvpMatrix, *viewProjectionMatrix);
                    graphics->setUniformFloat(shader.alpha, 1.0f);

                    graphics->drawLines(mesh);
                },
                [&](const ProjectileRenderTypeMindgun&) {
                    // TODO: implement mindgun if anyone actually uses it
                },
                [&](const ProjectileRenderTypeNone&) {
                    // do nothing
                });
        }
    }

    void RenderService::drawShaderMesh(const ShaderMesh& mesh, const Matrix4f& matrix, float seaLevel, bool shaded, PlayerColorIndex playerColorIndex)
    {
        auto mvpMatrix = (*viewProjectionMatrix) * matrix;

        if (mesh.vertices || mesh.teamVertices)
        {
            const auto& textureShader = shaders->unitTexture;
            graphics->bindShader(textureShader.handle.get());
            graphics->setUniformMatrix(textureShader.mvpMatrix, mvpMatrix);
            graphics->setUniformMatrix(textureShader.modelMatrix, matrix);
            graphics->setUniformFloat(textureShader.seaLevel, seaLevel);
            graphics->setUniformBool(textureShader.shade, shaded);

            if (mesh.vertices)
            {
                graphics->bindTexture(unitTextureAtlas->get());
                graphics->drawTriangles(*mesh.vertices);
            }

            if (mesh.teamVertices)
            {
                graphics->bindTexture(unitTeamTextureAtlases->at(playerColorIndex.value).get());
                graphics->drawTriangles(*mesh.teamVertices);
            }
        }
    }

    void RenderService::drawBatch(const ColoredMeshBatch& batch, const Matrix4f& vpMatrix)
    {
        if (!batch.lines.empty() || !batch.triangles.empty())
        {
            graphics->disableDepthBuffer();

            const auto& shader = shaders->basicColor;
            graphics->bindShader(shader.handle.get());
            graphics->setUniformFloat(shader.alpha, 1.0f);
            graphics->setUniformMatrix(shader.mvpMatrix, vpMatrix);
            if (!batch.lines.empty())
            {

                auto mesh = graphics->createColoredMesh(batch.lines, GL_STREAM_DRAW);
                graphics->drawLines(mesh);
            }

            if (!batch.triangles.empty())
            {
                auto mesh = graphics->createColoredMesh(batch.triangles, GL_STREAM_DRAW);
                graphics->drawTriangles(mesh);
            }

            graphics->enableDepthBuffer();
        }
    }

    void RenderService::drawUnitMeshBatch(const UnitMeshBatch& batch, float seaLevel, float time)
    {
        if (!batch.buildingMeshes.empty())
        {
            const auto& buildShader = shaders->unitBuild;
            graphics->bindShader(buildShader.handle.get());
            graphics->setUniformFloat(buildShader.seaLevel, seaLevel);
            graphics->setUniformFloat(buildShader.time, time);
            for (const auto& m : batch.buildingMeshes)
            {
                graphics->setUniformMatrix(buildShader.mvpMatrix, m.mvpMatrix);
                graphics->setUniformMatrix(buildShader.modelMatrix, m.modelMatrix);
                graphics->setUniformFloat(buildShader.unitY, m.unitY);
                graphics->setUniformBool(buildShader.shade, m.shaded);
                graphics->setUniformFloat(buildShader.percentComplete, m.percentComplete);

                graphics->bindTexture(m.texture);
                graphics->drawTriangles(*m.mesh);
            }
        }

        if (!batch.meshes.empty())
        {
            const auto& textureShader = shaders->unitTexture;
            graphics->bindShader(textureShader.handle.get());
            graphics->setUniformFloat(textureShader.seaLevel, seaLevel);
            for (const auto& m : batch.meshes)
            {
                graphics->setUniformMatrix(textureShader.mvpMatrix, m.mvpMatrix);
                graphics->setUniformMatrix(textureShader.modelMatrix, m.modelMatrix);
                graphics->setUniformBool(textureShader.shade, m.shaded);
                graphics->bindTexture(m.texture);
                graphics->drawTriangles(*m.mesh);
            }
        }
    }

    void RenderService::drawUnitShadowMeshBatch(const UnitShadowMeshBatch& batch)
    {
        if (batch.meshes.empty())
        {
            return;
        }

        graphics->enableStencilBuffer();
        graphics->clearStencilBuffer();
        graphics->useStencilBufferForWrites();
        graphics->disableColorBuffer();

        const auto& shader = shaders->unitShadow;
        graphics->bindShader(shader.handle.get());

        for (const auto& m : batch.meshes)
        {
            graphics->setUniformFloat(shader.groundHeight, m.groundHeight);
            graphics->setUniformMatrix(shader.vpMatrix, m.vpMatrix);
            graphics->setUniformMatrix(shader.modelMatrix, m.modelMatrix);

            graphics->bindTexture(m.texture);
            graphics->drawTriangles(*m.mesh);
        }

        graphics->useStencilBufferAsMask();
        graphics->enableColorBuffer();

        fillScreen(0.0f, 0.0f, 0.0f, 0.5f);

        graphics->enableColorBuffer();
        graphics->disableStencilBuffer();
    }

    void RenderService::drawSpriteBatch(const SpriteBatch& batch)
    {
        const auto& shader = shaders->basicTexture;
        graphics->bindShader(shader.handle.get());

        for (const auto& s : batch.sprites)
        {
            float alpha = s.translucent ? 0.5f : 1.0f;
            graphics->bindTexture(s.sprite->texture.get());
            graphics->setUniformMatrix(shader.mvpMatrix, s.mvpMatrix);
            graphics->setUniformVec4(shader.tint, 1.0f, 1.0f, 1.0f, alpha);
            graphics->drawTriangles(*s.sprite->mesh);
        }
    }
}
