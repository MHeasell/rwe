#include "RenderService.h"

namespace rwe
{
    RenderService::RenderService(
        GraphicsContext* graphics,
        ShaderService* shaders,
        const Matrix4f* viewProjectionMatrix)
        : graphics(graphics),
          shaders(shaders),
          viewProjectionMatrix(viewProjectionMatrix)
    {
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

    void RenderService::drawFlashes(GameTime currentTime, const std::vector<FlashEffect>& flashes)
    {
        const auto& shader = shaders->flashEffect;
        graphics->bindShader(shader.handle.get());

        auto quadMesh = graphics->createUnitTexturedQuad(Rectangle2f::fromTLBR(1.0f, 0.0f, 0.0f, 1.0f));

        graphics->unbindTexture();

        for (const auto& flash : flashes)
        {
            auto fractionComplete = flash.getFractionComplete(currentTime);
            if (!fractionComplete)
            {
                continue;
            }

            Vector3f snappedPosition(
                std::round(flash.position.x),
                truncateToInterval(flash.position.y, 2.0f),
                std::round(flash.position.z));

            // Convert to a model position that makes sense in the game world.
            // For standing (blocking) features we stretch y-dimension values by 2x
            // to correct for TA camera distortion.
            Matrix4f conversionMatrix = Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f));

            auto fractionRemaining = 1.0f - *fractionComplete;

            auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * Matrix4f::scale(flash.maxRadius * fractionRemaining);

            graphics->setUniformMatrix(shader.mvpMatrix, (*viewProjectionMatrix) * modelMatrix);
            graphics->setUniformFloat(shader.intensity, flash.maxIntensity * fractionRemaining);
            graphics->setUniformVec3(shader.color, flash.color.x, flash.color.y, flash.color.z);
            graphics->drawTriangles(quadMesh);
        }
    }

    void RenderService::drawBatch(const ColoredMeshBatch& batch, const Matrix4f& vpMatrix)
    {
        if (!batch.lines.empty() || !batch.triangles.empty())
        {
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
    void RenderService::drawLineLoopsBatch(const ColoredMeshesBatch& batch)
    {
        const auto& shader = shaders->basicColor;
        graphics->bindShader(shader.handle.get());
        for (const auto& m : batch.meshes)
        {
            graphics->setUniformMatrix(shader.mvpMatrix, m.mvpMatrix);
            graphics->setUniformFloat(shader.alpha, 1.0f);
            graphics->drawLineLoop(*m.mesh);
        }
    }
}
