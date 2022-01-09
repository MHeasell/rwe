#include "GameScene.h"
#include <algorithm>
#include <boost/range/adaptor/map.hpp>
#include <fstream>
#include <functional>
#include <rwe/CroppedViewport.h>
#include <rwe/GameScene_util.h>
#include <rwe/Index.h>
#include <rwe/Mesh.h>
#include <rwe/camera_util.h>
#include <rwe/dump_util.h>
#include <rwe/io/featuretdf/FeatureTdf.h>
#include <rwe/match.h>
#include <rwe/matrix_util.h>
#include <rwe/resource_io.h>
#include <rwe/sim/SimTicksPerSecond.h>
#include <rwe/ui/UiStagedButton.h>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace rwe
{
    Line3x<SimScalar> floatToSimLine(const Line3f& line)
    {
        return Line3x<SimScalar>(floatToSimVector(line.start), floatToSimVector(line.end));
    }

    bool projectileCollides(const GameSimulation& sim, const Projectile& projectile, const OccupiedCell& cellValue)
    {
        auto collidesWithOccupiedCell = match(
            cellValue.occupiedType,
            [&](const OccupiedUnit& v) {
                const auto& unit = sim.getUnit(v.id);

                if (unit.isOwnedBy(projectile.owner))
                {
                    return false;
                }

                // ignore if the projectile is above or below the unit
                if (projectile.position.y < unit.position.y || projectile.position.y > unit.position.y + unit.height)
                {
                    return false;
                }

                return true;
            },
            [&](const OccupiedFeature& v) {
                const auto& feature = sim.getFeature(v.id);

                // ignore if the projectile is above or below the feature
                if (projectile.position.y < feature.position.y || projectile.position.y > feature.position.y + feature.height)
                {
                    return false;
                }

                return true;
            },

            [&](const OccupiedNone&) {
                return false;
            });

        if (collidesWithOccupiedCell)
        {
            return true;
        }

        if (cellValue.buildingCell && !cellValue.buildingCell->passable)
        {
            const auto& unit = sim.getUnit(cellValue.buildingCell->unit);

            if (unit.isOwnedBy(projectile.owner))
            {
                return false;
            }

            // ignore if the projectile is above or below the unit
            if (projectile.position.y < unit.position.y || projectile.position.y > unit.position.y + unit.height)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    Matrix4f computeView(const Vector3f& cameraPosition)
    {
        auto translation = Matrix4f::translation(-cameraPosition);
        auto rotation = Matrix4f::rotationToAxes(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f));
        return rotation * translation;
    }

    Matrix4f computeInverseView(const Vector3f& cameraPosition)
    {
        auto translation = Matrix4f::translation(cameraPosition);
        auto rotation = Matrix4f::rotationToAxes(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, -1.0f), Vector3f(0.0f, 1.0f, 0.0f)).transposed();
        return translation * rotation;
    }

    Matrix4f computeProjection(float width, float height)
    {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;

        auto cabinet = Matrix4f::cabinetProjection(0.0f, 0.5f);

        auto ortho = Matrix4f::orthographicProjection(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            -1000.0f,
            1000.0f);

        return ortho * cabinet;
    }

    Matrix4f computeInverseProjection(float width, float height)
    {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;

        auto inverseCabinet = Matrix4f::cabinetProjection(0.0f, -0.5f);

        auto inverseOrtho = Matrix4f::inverseOrthographicProjection(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            -1000.0f,
            1000.0f);

        return inverseCabinet * inverseOrtho;
    }

    Matrix4f computeViewProjectionMatrix(const GameCameraState& cameraState, int screenWidth, int screenHeight)
    {
        auto view = computeView(cameraState.getRoundedPosition());
        auto projection = computeProjection(cameraState.scaleDimension(screenWidth), cameraState.scaleDimension(screenHeight));
        return projection * view;
    }

    Matrix4f computeInverseViewProjectionMatrix(const GameCameraState& cameraState, int screenWidth, int screenHeight)
    {
        auto inverseView = computeInverseView(cameraState.getRoundedPosition());
        auto inverseProjection = computeInverseProjection(cameraState.scaleDimension(screenWidth), cameraState.scaleDimension(screenHeight));
        return inverseView * inverseProjection;
    }

    const Rectangle2f GameScene::minimapViewport = Rectangle2f::fromTopLeft(0.0f, 0.0f, GuiSizeLeft, GuiSizeLeft);

    GameScene::GameScene(
        const SceneContext& sceneContext,
        std::unique_ptr<PlayerCommandService>&& playerCommandService,
        MeshDatabase&& meshDatabase,
        const GameCameraState& cameraState,
        SharedTextureHandle unitTextureAtlas,
        std::vector<SharedTextureHandle>&& unitTeamTextureAtlases,
        GameSimulation&& simulation,
        MapTerrainGraphics&& terrainGraphics,
        MovementClassCollisionService&& collisionService,
        UnitDatabase&& unitDatabase,
        std::unordered_map<std::string, FeatureTdf>&& featuresMap,
        MeshService&& meshService,
        std::unique_ptr<GameNetworkService>&& gameNetworkService,
        const std::shared_ptr<Sprite>& minimap,
        const std::shared_ptr<SpriteSeries>& minimapDots,
        const std::shared_ptr<Sprite>& minimapDotHighlight,
        InGameSoundsInfo sounds,
        const std::shared_ptr<SpriteSeries>& guiFont,
        PlayerId localPlayerId,
        TdfBlock* audioLookup,
        std::optional<std::ofstream>&& stateLogStream)
        : sceneContext(sceneContext),
          worldViewport(CroppedViewport(this->sceneContext.viewport, GuiSizeLeft, GuiSizeTop, GuiSizeRight, GuiSizeBottom)),
          playerCommandService(std::move(playerCommandService)),
          worldCameraState(cameraState),
          meshDatabase(std::move(meshDatabase)),
          unitTextureAtlas(unitTextureAtlas),
          unitTeamTextureAtlases(std::move(unitTeamTextureAtlases)),
          worldUiRenderService(this->sceneContext.graphics, this->sceneContext.shaders, &this->worldViewport),
          chromeUiRenderService(this->sceneContext.graphics, this->sceneContext.shaders, this->sceneContext.viewport),
          simulation(std::move(simulation)),
          terrainGraphics(std::move(terrainGraphics)),
          collisionService(std::move(collisionService)),
          unitDatabase(std::move(unitDatabase)),
          unitFactory(sceneContext.textureService, &this->unitDatabase, std::move(meshService), &this->collisionService, sceneContext.palette, sceneContext.guiPalette, &this->simulation),
          featuresMap(std::move(featuresMap)),
          gameNetworkService(std::move(gameNetworkService)),
          pathFindingService(&this->simulation, &this->collisionService),
          cobExecutionService(),
          unitBehaviorService(this, &this->unitFactory, &this->cobExecutionService),
          minimap(minimap),
          minimapDots(minimapDots),
          minimapDotHighlight(minimapDotHighlight),
          minimapRect(minimapViewport.scaleToFit(this->minimap->bounds)),
          sounds(std::move(sounds)),
          guiFont(guiFont),
          localPlayerId(localPlayerId),
          uiFactory(sceneContext.textureService, sceneContext.audioService, audioLookup, sceneContext.vfs, sceneContext.pathMapping, sceneContext.viewport->width(), sceneContext.viewport->height()),
          stateLogStream(std::move(stateLogStream))
    {
    }

    void GameScene::init()
    {
        const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
        currentPanel = uiFactory.panelFromGuiFile(sidePrefix + "MAIN2");

        sceneContext.audioService->reserveChannels(reservedChannelsCount);
        gameNetworkService->start();

        recreateWorldRenderTextures();
    }

    float computeSoundCeiling(int soundCount)
    {
        assert(soundCount > 0);
        if (soundCount <= 4)
        {
            return soundCount;
        }

        if (soundCount <= 8)
        {
            return 4 + ((soundCount - 4) * 0.5f);
        }

        if (soundCount <= 16)
        {
            return (6 + ((soundCount - 8) * 0.25f));
        }

        return 8;
    }

    int computeSoundVolume(int soundCount)
    {
        soundCount = std::max(soundCount, 1);
        auto headRoom = computeSoundCeiling(soundCount);
        return std::clamp(static_cast<int>(headRoom * 128) / soundCount, 1, 128);
    }

    void GameScene::render()
    {
        if (guiVisible)
        {
            renderUi();
        }

        sceneContext.graphics->enableDepthBuffer();

        renderWorld();
        sceneContext.graphics->disableDepthBuffer();

        // oh yeah also regulate sound
        std::scoped_lock<std::mutex> lock(playingUnitChannelsLock);
        auto volume = computeSoundVolume(playingUnitChannels.size());
        for (auto channel : playingUnitChannels)
        {
            sceneContext.audioService->setVolume(channel, volume);
        }
    }

    void GameScene::renderUi()
    {
        renderMinimap();

        const auto& localSideData = sceneContext.sideData->at(getPlayer(localPlayerId).side);

        // render top bar
        const auto& intGafName = localSideData.intGaf;
        auto topPanelBackground = sceneContext.textureService->tryGetGafEntry("anims/" + intGafName + ".GAF", "PANELTOP");
        auto bottomPanelBackground = sceneContext.textureService->tryGetGafEntry("anims/" + intGafName + ".GAF", "PANELBOT");
        float topXBuffer = GuiSizeLeft;
        if (topPanelBackground)
        {
            const auto& sprite = *(*topPanelBackground)->sprites.at(0);
            chromeUiRenderService.drawSpriteAbs(topXBuffer, 0, sprite);
            topXBuffer += sprite.bounds.width();
        }
        if (bottomPanelBackground)
        {
            while (topXBuffer < sceneContext.viewport->width())
            {
                const auto& sprite = *(*bottomPanelBackground)->sprites.at(0);
                chromeUiRenderService.drawSpriteAbs(topXBuffer, 0.0f, sprite);
                topXBuffer += sprite.bounds.width();
            }
        }

        auto logos = sceneContext.textureService->tryGetGafEntry("textures/LOGOS.GAF", "32xlogos");
        if (logos)
        {
            auto playerColorIndex = getPlayer(localPlayerId).color;
            const auto& rect = localSideData.logo.toDiscreteRect();
            chromeUiRenderService.drawSpriteAbs(rect.x, rect.y, rect.width, rect.height, *(*logos)->sprites.at(playerColorIndex.value));
        }

        // draw energy bar
        {
            const auto& rect = localSideData.energyBar.toDiscreteRect();
            const auto& localPlayer = getPlayer(localPlayerId);
            auto rectWidth = (rect.width * std::max(Energy(0), localPlayer.energy).value) / localPlayer.maxEnergy.value;
            const auto& colorIndex = localSideData.energyColor;
            const auto& color = sceneContext.palette->at(colorIndex);
            chromeUiRenderService.fillColor(rect.x, rect.y, rectWidth, rect.height, color);
        }
        {
            const auto& rect = localSideData.energy0;
            chromeUiRenderService.drawText(rect.x1, rect.y1, formatResource(Energy(0)), *guiFont);
        }
        {
            const auto& rect = localSideData.energyMax;
            auto text = formatResource(getPlayer(localPlayerId).maxEnergy);
            chromeUiRenderService.drawTextAlignRight(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.energyNum;
            auto text = formatResource(std::max(Energy(0), getPlayer(localPlayerId).energy));
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.energyProduced;
            auto text = formatResourceDelta(getPlayer(localPlayerId).energyProductionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(83, 223, 79));
        }
        {
            const auto& rect = localSideData.energyConsumed;
            auto text = formatResourceDelta(getPlayer(localPlayerId).previousDesiredEnergyConsumptionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(255, 71, 0));
        }

        // draw metal bar
        {
            const auto& rect = localSideData.metalBar.toDiscreteRect();
            const auto& localPlayer = getPlayer(localPlayerId);
            auto rectWidth = (rect.width * std::max(Metal(0), localPlayer.metal).value) / localPlayer.maxMetal.value;
            const auto& colorIndex = localSideData.metalColor;
            const auto& color = sceneContext.palette->at(colorIndex);
            chromeUiRenderService.fillColor(rect.x, rect.y, rectWidth, rect.height, color);
        }
        {
            const auto& rect = localSideData.metal0;
            chromeUiRenderService.drawText(rect.x1, rect.y1, "0", *guiFont);
        }
        {
            const auto& rect = localSideData.metalMax;
            auto text = formatResource(getPlayer(localPlayerId).maxMetal);
            chromeUiRenderService.drawTextAlignRight(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.metalNum;
            auto text = formatResource(std::max(Metal(0), getPlayer(localPlayerId).metal));
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.metalProduced;
            auto text = formatResourceDelta(getPlayer(localPlayerId).metalProductionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(83, 223, 79));
        }
        {
            const auto& rect = localSideData.metalConsumed;
            auto text = formatResourceDelta(getPlayer(localPlayerId).previousDesiredMetalConsumptionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(255, 71, 0));
        }

        // render bottom bar
        float bottomXBuffer = GuiSizeLeft;
        if (bottomPanelBackground)
        {
            while (bottomXBuffer < sceneContext.viewport->width())
            {
                const auto& sprite = *(*bottomPanelBackground)->sprites.at(0);
                chromeUiRenderService.drawSpriteAbs(bottomXBuffer, worldViewport.bottom(), sprite);
                bottomXBuffer += sprite.bounds.width();
            }
        }

        auto extraBottom = sceneContext.viewport->height() - 480;
        if (hoveredUnit)
        {
            const auto& unit = getUnit(*hoveredUnit);
            if (logos)
            {
                const auto& rect = localSideData.logo2.toDiscreteRect();
                const auto& color = *(*logos)->sprites.at(getPlayer(unit.owner).color.value);
                chromeUiRenderService.drawSpriteAbs(rect.x, extraBottom + rect.y, rect.width, rect.height, color);
            }

            {
                const auto& rect = localSideData.unitName;
                const auto& playerName = getPlayer(unit.owner).name;
                const auto& text = unit.showPlayerName && playerName ? *playerName : unit.name;
                chromeUiRenderService.drawTextCenteredX(rect.x1, extraBottom + rect.y1, text, *guiFont);
            }

            if (unit.isOwnedBy(localPlayerId) || !unit.hideDamage)
            {
                const auto& rect = localSideData.damageBar.toDiscreteRect();
                chromeUiRenderService.drawHealthBar2(rect.x, extraBottom + rect.y, rect.width, rect.height, static_cast<float>(unit.hitPoints) / static_cast<float>(unit.maxHitPoints));
            }

            if (unit.isOwnedBy(localPlayerId))
            {
                {
                    const auto& rect = localSideData.unitMetalMake;
                    auto text = "+" + formatResourceDelta(unit.getMetalMake());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(83, 223, 79));
                }
                {
                    const auto& rect = localSideData.unitMetalUse;
                    auto text = "-" + formatResourceDelta(unit.getMetalUse());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(255, 71, 0));
                }
                {
                    const auto& rect = localSideData.unitEnergyMake;
                    auto text = "+" + formatResourceDelta(unit.getEnergyMake());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(83, 223, 79));
                }
                {
                    const auto& rect = localSideData.unitEnergyUse;
                    auto text = "-" + formatResourceDelta(unit.getEnergyUse());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(255, 71, 0));
                }
                {
                    const auto& rect = localSideData.missionText;
                    auto text = "Standby";
                    chromeUiRenderService.drawTextCenteredX(rect.x1, extraBottom + rect.y1, text, *guiFont);
                }
            }
        }

        currentPanel->render(chromeUiRenderService);
    }

    void GameScene::renderMinimap()
    {
        // draw minimap
        chromeUiRenderService.drawSpriteAbs(minimapRect, *minimap);

        auto cameraInverse = computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        auto worldToMinimap = worldToMinimapMatrix(simulation.terrain, minimapRect);

        // draw minimap dots
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            auto minimapPos = worldToMinimap * simVectorToFloat(unit.position);
            minimapPos.x = std::floor(minimapPos.x);
            minimapPos.y = std::floor(minimapPos.y);
            auto ownerId = unit.owner;
            auto colorIndex = getPlayer(ownerId).color;
            chromeUiRenderService.drawSprite(minimapPos.x, minimapPos.y, *minimapDots->sprites[colorIndex.value]);
        }
        // highlight the minimap dot for the hovered unit
        if (hoveredUnit)
        {
            const auto& unit = getUnit(*hoveredUnit);
            auto minimapPos = worldToMinimap * simVectorToFloat(unit.position);
            minimapPos.x = std::floor(minimapPos.x);
            minimapPos.y = std::floor(minimapPos.y);
            chromeUiRenderService.drawSprite(minimapPos.x, minimapPos.y, *minimapDotHighlight);
        }

        // draw minimap viewport rectangle
        {
            auto transform = worldToMinimap * cameraInverse;
            auto bottomLeft = transform * Vector3f(-1.0f, -1.0f, 0.0f);
            auto topRight = transform * Vector3f(1.0f, 1.0f, 0.0f);

            chromeUiRenderService.drawBoxOutline(
                std::round(bottomLeft.x),
                std::round(topRight.y),
                std::round(topRight.x - bottomLeft.x),
                std::round(bottomLeft.y - topRight.y),
                Color(247, 227, 103));
        }
    }

    void GameScene::renderBuildBoxes(const Unit& unit, const Color& color)
    {
        auto worldToUi = worldUiRenderService.getInverseViewProjectionMatrix()
            * computeViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        for (const auto& order : unit.orders)
        {
            if (const auto buildOrder = std::get_if<BuildOrder>(&order))
            {
                const auto& unitType = buildOrder->unitType;
                auto mc = unitFactory.getAdHocMovementClass(unitType);
                auto footprint = unitFactory.getUnitFootprint(unitType);
                auto footprintRect = computeFootprintRegion(buildOrder->position, footprint.x, footprint.y);

                auto topLeftWorld = simulation.terrain.heightmapIndexToWorldCorner(footprintRect.x, footprintRect.y);
                topLeftWorld.y = simulation.terrain.getHeightAt(
                    topLeftWorld.x + ((SimScalar(footprintRect.width) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss),
                    topLeftWorld.z + ((SimScalar(footprintRect.height) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss));

                auto topLeftUi = worldToUi * simVectorToFloat(topLeftWorld);
                worldUiRenderService.drawBoxOutline(
                    topLeftUi.x,
                    topLeftUi.y,
                    footprintRect.width * simScalarToFloat(MapTerrain::HeightTileWidthInWorldUnits),
                    footprintRect.height * simScalarToFloat(MapTerrain::HeightTileHeightInWorldUnits),
                    color,
                    2.0f);
            }
        }
    }

    void GameScene::renderUnitOrders(UnitId unitId, bool drawLines)
    {
        const auto& unit = getUnit(unitId);
        auto pos = unit.position;
        auto worldToUi = worldUiRenderService.getInverseViewProjectionMatrix()
            * computeViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        for (const auto& order : unit.orders)
        {
            auto nextPos = match(
                order,
                [&](const BuildOrder& o) { return o.position; },
                [&](const MoveOrder& o) { return o.destination; },
                [&](const AttackOrder& o) { return match(
                                                o.target,
                                                [&](const SimVector& v) { return v; },
                                                [&](const UnitId& u) {
                                                    auto unitOption = tryGetUnit(u);
                                                    if (!unitOption)
                                                    {
                                                        return pos;
                                                    }
                                                    return unitOption->get().position;
                                                }); },
                [&](const BuggerOffOrder&) { return pos; },
                [&](const CompleteBuildOrder& o) {
                    auto unitOption = tryGetUnit(o.target);
                    if (!unitOption)
                    {
                        return pos;
                    }
                    return unitOption->get().position;
                },
                [&](const GuardOrder& o) {
                    auto unitOption = tryGetUnit(o.target);
                    if (!unitOption)
                    {
                        return pos;
                    }
                    return unitOption->get().position;
                });

            auto waypointIcon = match(
                order,
                [&](const BuildOrder&) { return std::optional<CursorType>(); },
                [&](const MoveOrder&) { return std::optional<CursorType>(CursorType::Move); },
                [&](const AttackOrder&) { return std::optional<CursorType>(CursorType::Attack); },
                [&](const BuggerOffOrder&) { return std::optional<CursorType>(); },
                [&](const CompleteBuildOrder&) { return std::optional<CursorType>(CursorType::Repair); },
                [&](const GuardOrder&) { return std::optional<CursorType>(CursorType::Guard); });

            // draw waypoint icons
            if (waypointIcon)
            {
                auto timeInMillis = sceneContext.timeService->getTicks();
                unsigned int frameRateInSeconds = 2;
                unsigned int millisPerFrame = 1000 / frameRateInSeconds;

                const auto& frames = sceneContext.cursor->getCursor(*waypointIcon)->sprites;
                auto frameIndex = (timeInMillis / millisPerFrame) % frames.size();

                auto uiDest = worldToUi * simVectorToFloat(nextPos);
                worldUiRenderService.drawSprite(uiDest.x, uiDest.y, *(frames[frameIndex]), Color(255, 255, 255, 100));
            }

            if (drawLines)
            {
                auto drawLine = match(
                    order,
                    [&](const BuildOrder&) { return true; },
                    [&](const MoveOrder&) { return true; },
                    [&](const AttackOrder&) { return false; },
                    [&](const BuggerOffOrder&) { return false; },
                    [&](const CompleteBuildOrder&) { return true; },
                    [&](const GuardOrder&) { return true; });

                if (drawLine)
                {
                    auto uiPos = worldToUi * simVectorToFloat(pos);
                    auto uiDest = worldToUi * simVectorToFloat(nextPos);
                    worldUiRenderService.drawLine(uiPos.xy(), uiDest.xy());
                }
            }

            pos = nextPos;
        }
    }

    void GameScene::renderWorld()
    {
        sceneContext.graphics->bindFrameBuffer(worldFrameBuffer.frameBuffer.get());
        sceneContext.graphics->setViewport(
            0,
            0,
            worldViewport.width(),
            worldViewport.height());

        sceneContext.graphics->clear();

        const auto& viewProjectionMatrix = computeViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        RenderService worldRenderService(sceneContext.graphics, sceneContext.shaders, &meshDatabase, &unitDatabase, &viewProjectionMatrix, &unitTextureAtlas, &unitTeamTextureAtlases);

        sceneContext.graphics->disableDepthBuffer();

        worldRenderService.drawMapTerrain(terrainGraphics, worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));

        SpriteBatch flatFeatureBatch;
        SpriteBatch flatFeatureShadowBatch;
        for (const auto& f : simulation.features)
        {
            if (!f.second.isStanding())
            {
                drawFeature(f.second, viewProjectionMatrix, flatFeatureBatch);
                drawFeatureShadow(f.second, viewProjectionMatrix, flatFeatureShadowBatch);
            }
        }
        worldRenderService.drawSpriteBatch(flatFeatureShadowBatch);
        worldRenderService.drawSpriteBatch(flatFeatureBatch);

        sceneContext.graphics->enableDepthBuffer();

        ColoredMeshBatch terrainOverlayBatch;

        if (occupiedGridVisible)
        {
            drawOccupiedGrid(worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()), simulation.terrain, simulation.occupiedGrid, terrainOverlayBatch);
        }
        if (pathfindingVisualisationVisible)
        {
            drawPathfindingVisualisation(simulation.terrain, pathFindingService.lastPathDebugInfo, terrainOverlayBatch);
        }

        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && movementClassGridVisible)
        {
            const auto& unit = simulation.getUnit(*selectedUnit);
            if (unit.movementClass)
            {
                const auto& grid = collisionService.getGrid(*unit.movementClass);
                drawMovementClassCollisionGrid(simulation.terrain, grid, worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()), terrainOverlayBatch);
            }
        }

        worldRenderService.drawBatch(terrainOverlayBatch, viewProjectionMatrix);

        sceneContext.graphics->disableDepthBuffer();

        auto interpolationFraction = static_cast<float>(millisecondsBuffer) / static_cast<float>(SimMillisecondsPerTick);
        for (const auto& selectedUnitId : selectedUnits)
        {
            const auto& unit = getUnit(selectedUnitId);
            worldRenderService.drawSelectionRect(unit, interpolationFraction);
        }

        auto seaLevel = simulation.terrain.getSeaLevel();

        UnitShadowMeshBatch unitShadowMeshBatch;
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            auto groundHeight = simulation.terrain.getHeightAt(unit.position.x, unit.position.z);
            if (unit.floater || unit.canHover)
            {
                groundHeight = rweMax(groundHeight, seaLevel);
            }
            drawUnitShadow(&unitDatabase, meshDatabase, viewProjectionMatrix, unit, interpolationFraction, simScalarToFloat(groundHeight), unitTextureAtlas.get(), unitTeamTextureAtlases, unitShadowMeshBatch);
        }
        for (const auto& feature : (simulation.features | boost::adaptors::map_values))
        {
            const auto& position = feature.position;
            auto groundHeight = simulation.terrain.getHeightAt(position.x, position.z);
            if (position.y >= seaLevel && groundHeight < seaLevel)
            {
                groundHeight = seaLevel;
            }
            drawFeatureMeshShadow(&unitDatabase, meshDatabase, viewProjectionMatrix, feature, simScalarToFloat(groundHeight), unitTextureAtlas.get(), unitTeamTextureAtlases, unitShadowMeshBatch);
        }
        worldRenderService.drawUnitShadowMeshBatch(unitShadowMeshBatch);

        sceneContext.graphics->enableDepthBuffer();

        UnitMeshBatch unitMeshBatch;
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            drawUnit(&unitDatabase, meshDatabase, viewProjectionMatrix, unit, getPlayer(unit.owner).color, interpolationFraction, unitTextureAtlas.get(), unitTeamTextureAtlases, unitMeshBatch);
        }
        for (const auto& feature : (simulation.features | boost::adaptors::map_values))
        {
            drawMeshFeature(&unitDatabase, meshDatabase, viewProjectionMatrix, feature, unitTextureAtlas.get(), unitTeamTextureAtlases, unitMeshBatch);
        }
        worldRenderService.drawUnitMeshBatch(unitMeshBatch, simScalarToFloat(seaLevel), simulation.gameTime.value);

        worldRenderService.drawProjectiles(simulation.projectiles, simScalarToFloat(seaLevel), simulation.gameTime, interpolationFraction);

        sceneContext.graphics->disableDepthWrites();

        SpriteBatch featureBatch;
        SpriteBatch featureShadowBatch;
        for (const auto& f : simulation.features)
        {
            if (f.second.isStanding())
            {
                drawFeature(f.second, viewProjectionMatrix, featureBatch);
                drawFeatureShadow(f.second, viewProjectionMatrix, featureShadowBatch);
            }
        }
        worldRenderService.drawSpriteBatch(featureShadowBatch);
        worldRenderService.drawSpriteBatch(featureBatch);

        sceneContext.graphics->disableDepthTest();
        ColoredMeshBatch nanoLinesBatch;
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            if (auto nanolatheTarget = unit.getActiveNanolatheTarget())
            {
                auto targetUnitOption = tryGetUnit(nanolatheTarget->first);
                if (targetUnitOption)
                {
                    drawNanoLine(simVectorToFloat(nanolatheTarget->second), simVectorToFloat(targetUnitOption->get().position), nanoLinesBatch);
                }
            }
        }
        worldRenderService.drawBatch(nanoLinesBatch, viewProjectionMatrix);

        sceneContext.graphics->bindFrameBufferColorBuffer(dodgeMask.get());
        sceneContext.graphics->clearColor();
        worldRenderService.drawFlashes(simulation.gameTime, flashes);
        sceneContext.graphics->bindFrameBufferColorBuffer(worldFrameBuffer.texture.get());

        sceneContext.graphics->unbindFrameBuffer();
        auto viewportPos = worldViewport.toOtherViewport(*sceneContext.viewport, 0, worldViewport.height());
        sceneContext.graphics->setViewport(
            viewportPos.x,
            sceneContext.viewport->height() - viewportPos.y,
            worldViewport.width(),
            worldViewport.height());

        sceneContext.graphics->disableDepthBuffer();
        auto quadMesh = sceneContext.graphics->createUnitTexturedQuadFlipped(Rectangle2f::fromTLBR(1.0f, 0.0f, 0.0f, 1.0f));
        sceneContext.graphics->bindShader(sceneContext.shaders->worldPost.handle.get());
        sceneContext.graphics->setUniformInt(sceneContext.shaders->worldPost.dodgeMask, 1);
        sceneContext.graphics->bindTexture(worldFrameBuffer.texture.get());
        sceneContext.graphics->setActiveTextureSlot1();
        sceneContext.graphics->bindTexture(dodgeMask.get());
        sceneContext.graphics->setActiveTextureSlot0();
        sceneContext.graphics->drawTriangles(quadMesh);

        SpriteBatch explosionsBatch;
        for (const auto& exp : explosions)
        {
            drawExplosion(meshDatabase, simulation.gameTime, viewProjectionMatrix, exp, explosionsBatch);
        }
        worldRenderService.drawSpriteBatch(explosionsBatch);
        sceneContext.graphics->enableDepthTest();

        sceneContext.graphics->enableDepthWrites();

        // in-world UI/overlay rendering
        if (isShiftDown())
        {
            auto singleSelectedUnit = getSingleSelectedUnit();

            // if unit is a builder, show all other buildings being built
            if ((singleSelectedUnit && getUnit(*singleSelectedUnit).builder) || (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId) && getUnit(*hoveredUnit).builder))
            {
                for (const auto& [_, unit] : simulation.units)
                {
                    if (unit.isOwnedBy(localPlayerId))
                    {
                        renderBuildBoxes(unit, Color(0, 0, 255));
                    }
                }
            }

            // draw orders + lines for hovered unit
            if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
            {
                renderUnitOrders(*hoveredUnit, true);
            }

            // draw orders for all selected units
            for (const auto& selectedUnitId : selectedUnits)
            {
                renderBuildBoxes(getUnit(selectedUnitId), Color(0, 255, 0));

                if (selectedUnitId != hoveredUnit)
                {
                    // draw lines if only one unit is selected--hovered unit is drawn aleady
                    renderUnitOrders(selectedUnitId, singleSelectedUnit == selectedUnitId);
                }
            }
        }

        if (healthBarsVisible)
        {
            for (const Unit& unit : (simulation.units | boost::adaptors::map_values))
            {
                if (!unit.isOwnedBy(localPlayerId))
                {
                    // only draw healthbars on units we own
                    continue;
                }

                if (unit.hitPoints == 0)
                {
                    // Do not show health bar when the unit has zero health.
                    // This can happen when the unit is still a freshly created nanoframe.
                    continue;
                }

                auto uiPos = worldUiRenderService.getInverseViewProjectionMatrix()
                    * viewProjectionMatrix
                    * simVectorToFloat(unit.position);
                worldUiRenderService.drawHealthBar(uiPos.x, uiPos.y, static_cast<float>(unit.hitPoints) / static_cast<float>(unit.maxHitPoints));
            }
        }

        // Draw build box outline when a unit is selected to be built
        if (hoverBuildInfo)
        {
            Color color = hoverBuildInfo->isValid ? Color(0, 255, 0) : Color(255, 0, 0);

            auto topLeftWorld = simulation.terrain.heightmapIndexToWorldCorner(hoverBuildInfo->rect.x, hoverBuildInfo->rect.y);
            topLeftWorld.y = simulation.terrain.getHeightAt(
                topLeftWorld.x + ((SimScalar(hoverBuildInfo->rect.width) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss),
                topLeftWorld.z + ((SimScalar(hoverBuildInfo->rect.height) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss));

            auto topLeftUi = worldUiRenderService.getInverseViewProjectionMatrix()
                * viewProjectionMatrix
                * simVectorToFloat(topLeftWorld);
            worldUiRenderService.drawBoxOutline(
                topLeftUi.x,
                topLeftUi.y,
                hoverBuildInfo->rect.width * simScalarToFloat(MapTerrain::HeightTileWidthInWorldUnits),
                hoverBuildInfo->rect.height * simScalarToFloat(MapTerrain::HeightTileHeightInWorldUnits),
                color,
                2.0f);
        }

        // Draw bandbox selection rectangle
        if (auto normalCursorMode = std::get_if<NormalCursorMode>(&cursorMode.getValue()))
        {
            if (auto selectingState = std::get_if<NormalCursorMode::SelectingState>(&normalCursorMode->state))
            {
                const auto& start = selectingState->startPosition;
                const auto cameraPosition = worldCameraState.getRoundedPosition();
                Point cameraRelativeStart(start.x - cameraPosition.x, start.y - cameraPosition.z);

                auto worldViewportPos = sceneContext.viewport->toOtherViewport(worldViewport, getMousePosition());
                auto rect = DiscreteRect::fromPoints(cameraRelativeStart, worldViewportPos);

                worldUiRenderService.drawBoxOutline(rect.x, rect.y, rect.width, rect.height, Color(255, 255, 255));
                if (rect.width > 2 && rect.height > 2)
                {
                    worldUiRenderService.drawBoxOutline(rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2, Color(0, 0, 0));
                }
            }
        }

        if (cursorTerrainDotVisible)
        {
            // draw a dot where we think the cursor intersects terrain
            auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
            auto intersect = simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));
            if (intersect)
            {
                auto cursorTerrainPos = worldUiRenderService.getInverseViewProjectionMatrix()
                    * viewProjectionMatrix
                    * simVectorToFloat(*intersect);
                worldUiRenderService.fillColor(cursorTerrainPos.x - 2, cursorTerrainPos.y - 2, 4, 4, Color(0, 0, 255));

                intersect->y = simulation.terrain.getHeightAt(intersect->x, intersect->z);

                auto heightTestedTerrainPos = worldUiRenderService.getInverseViewProjectionMatrix()
                    * viewProjectionMatrix
                    * simVectorToFloat(*intersect);
                worldUiRenderService.fillColor(heightTestedTerrainPos.x - 2, heightTestedTerrainPos.y - 2, 4, 4, Color(255, 0, 0));
            }
        }

        sceneContext.graphics->enableDepthBuffer();

        sceneContext.graphics->setViewport(0, 0, sceneContext.viewport->width(), sceneContext.viewport->height());
    }

    const char* stateToString(const UnitState& state)
    {
        return match(
            state,
            [&](const IdleState&) {
                return "idle";
            },
            [&](const BuildingState&) {
                return "building";
            },
            [&](const MovingState&) {
                return "moving";
            },
            [&](const CreatingUnitState&) {
                return "creating unit";
            });
    }

    const char* cobAxisToString(const CobAxis& axis)
    {
        switch (axis)
        {
            case CobAxis::X:
                return "x-axis";
            case CobAxis::Y:
                return "Y-axis";
            case CobAxis::Z:
                return "z-axis";
            default:
                throw std::logic_error("invalid axis");
        }
    }

    std::string blockedStatusToString(const CobEnvironment::BlockedStatus& status)
    {
        return match(
            status.condition,
            [&](const CobEnvironment::BlockedStatus::Move& m) {
                return "wait-for-move piece " + std::to_string(m.object) + " along " + cobAxisToString(m.axis);
            },
            [&](const CobEnvironment::BlockedStatus::Turn& t) {
                return "wait-for-turn piece " + std::to_string(t.object) + " around " + cobAxisToString(t.axis);
            });
    }

    void renderUnitInfoSection(const Unit& unit)
    {
        ImGui::LabelText("State", "%s", stateToString(unit.behaviourState));
        ImGui::Text("Cob vars");
        for (Index i = 0; i < getSize(unit.cobEnvironment->_statics); ++i)
        {
            ImGui::Text("%lld: %d", i, unit.cobEnvironment->_statics[i]);
        }

        ImGui::Text("Cob threads");
        for (Index i = 0; i < getSize(unit.cobEnvironment->threads); ++i)
        {
            const auto& thread = *unit.cobEnvironment->threads[i];
            ImGui::Text("%lld: %s (%u)", i, thread.name.c_str(), thread.signalMask);
        }

        ImGui::Text("ready threads");
        for (Index i = 0; i < getSize(unit.cobEnvironment->readyQueue); ++i)
        {
            ImGui::Text("%s", unit.cobEnvironment->readyQueue[i]->name.c_str());
        }

        ImGui::Text("blocked threads");
        for (Index i = 0; i < getSize(unit.cobEnvironment->blockedQueue); ++i)
        {
            const auto& pair = unit.cobEnvironment->blockedQueue[i];
            ImGui::Text("%s, %s", pair.second->name.c_str(), blockedStatusToString(pair.first).c_str());
        }

        ImGui::Text("sleeping threads");
        for (Index i = 0; i < getSize(unit.cobEnvironment->sleepingQueue); ++i)
        {
            const auto& pair = unit.cobEnvironment->sleepingQueue[i];
            ImGui::Text("%s, wake time: %d", pair.second->name.c_str(), pair.first.value);
        }

        ImGui::Text("finished threads");
        for (Index i = 0; i < getSize(unit.cobEnvironment->finishedQueue); ++i)
        {
            ImGui::Text("%s", unit.cobEnvironment->finishedQueue[i]->name.c_str());
        }

        ImGui::LabelText("Floater", "%s", unit.floater ? "true" : "false");
        ImGui::LabelText("x", "%f", unit.position.x.value);
        ImGui::LabelText("y", "%f", unit.position.y.value);
        ImGui::LabelText("z", "%f", unit.position.z.value);
    }

    void GameScene::renderDebugWindow()
    {
        if (!showDebugWindow)
        {
            return;
        }

        ImGui::Begin("Game Debug", &showDebugWindow);
        ImGui::Checkbox("Health bars", &healthBarsVisible);
        if (ImGui::Checkbox("GUI", &guiVisible))
        {
            if (guiVisible)
            {
                worldViewport.setInset(GuiSizeLeft, GuiSizeTop, GuiSizeRight, GuiSizeBottom);
            }
            else
            {
                worldViewport.setInset(0, 0, 0, 0);
            }
            recreateWorldRenderTextures();
        }
        ImGui::Separator();
        ImGui::Checkbox("Cursor terrain dot", &cursorTerrainDotVisible);
        ImGui::Checkbox("Occupied grid", &occupiedGridVisible);
        ImGui::Checkbox("Pathfinding visualisation", &pathfindingVisualisationVisible);
        ImGui::Checkbox("Movement class grid", &movementClassGridVisible);
        ImGui::InputInt("Side", &unitSpawnPlayer);
        if (ImGui::InputText("Spawn Unit", unitSpawnText, IM_ARRAYSIZE(unitSpawnText), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string text(unitSpawnText);
            if (!text.empty() && unitFactory.isValidUnitType(text) && unitSpawnPlayer >= 0 && unitSpawnPlayer < getSize(simulation.players))
            {
                if (auto terrainPos = getMouseTerrainCoordinate())
                {
                    spawnCompletedUnit(text, PlayerId(unitSpawnPlayer), *terrainPos);
                }
            }
            ImGui::SetKeyboardFocusHere(-1);
        }
        ImGui::Separator();
        {
            std::scoped_lock<std::mutex> lock(playingUnitChannelsLock);
            ImGui::LabelText("Unit sounds", "%lld", getSize(playingUnitChannels));
            ImGui::LabelText("Sound volume", "%d", computeSoundVolume(getSize(playingUnitChannels)));
        }

        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
        {
            const auto& unit = getUnit(*selectedUnit);

            ImGui::Separator();
            ImGui::Text("Selected Unit");
            renderUnitInfoSection(unit);
        }

        if (hoveredUnit)
        {
            const auto& unit = getUnit(*hoveredUnit);

            ImGui::Separator();
            ImGui::Text("Hovered Unit");
            renderUnitInfoSection(unit);
        }

        ImGui::End();
    }

    void GameScene::onKeyDown(const SDL_Keysym& keysym)
    {
        currentPanel->keyDown(KeyEvent(keysym.sym));

        if (keysym.sym == SDLK_UP)
        {
            up = true;
        }
        else if (keysym.sym == SDLK_DOWN)
        {
            down = true;
        }
        else if (keysym.sym == SDLK_LEFT)
        {
            left = true;
        }
        else if (keysym.sym == SDLK_RIGHT)
        {
            right = true;
        }
        else if (keysym.sym == SDLK_LCTRL)
        {
            leftCtrlDown = true;
        }
        else if (keysym.sym == SDLK_RCTRL)
        {
            rightCtrlDown = true;
        }
        else if (keysym.sym == SDLK_LSHIFT)
        {
            leftShiftDown = true;
        }
        else if (keysym.sym == SDLK_RSHIFT)
        {
            rightShiftDown = true;
        }
        else if (keysym.sym == SDLK_ESCAPE)
        {
            handleEscapeDown();
        }
        else if (keysym.sym == SDLK_F11)
        {
            showDebugWindow = true;
        }
        else if (keysym.scancode == SDL_SCANCODE_GRAVE)
        {
            healthBarsVisible = !healthBarsVisible;
        }
        else if (keysym.sym == SDLK_t)
        {
            startTrack();
        }
        else if (keysym.sym == SDLK_c)
        {
            if (isCtrlDown())
            {
                const auto& localSideData = sceneContext.sideData->at(getPlayer(localPlayerId).side);
                if (!isShiftDown())
                {
                    clearUnitSelection();
                }
                // Select commanders (edge case: debug mode allows spawning multiple commanders)
                for (const auto& [unitId, unit] : simulation.units)
                {
                    if (unit.isCommander() && unit.isOwnedBy(localPlayerId))
                    {
                        selectAdditionalUnit(unitId);
                        // For multiple commanders, OTA selects all but always tracks only the last spawned (it won't cycle with repeated ctrl-c)
                        trackedUnitId = unitId;
                    }
                }
                trackingOn = true;
            }
        }
    }

    void GameScene::onKeyUp(const SDL_Keysym& keysym)
    {
        currentPanel->keyUp(KeyEvent(keysym.sym));

        if (keysym.sym == SDLK_UP)
        {
            up = false;
        }
        else if (keysym.sym == SDLK_DOWN)
        {
            down = false;
        }
        else if (keysym.sym == SDLK_LEFT)
        {
            left = false;
        }
        else if (keysym.sym == SDLK_RIGHT)
        {
            right = false;
        }
        else if (keysym.sym == SDLK_LCTRL)
        {
            leftCtrlDown = false;
        }
        else if (keysym.sym == SDLK_RCTRL)
        {
            rightCtrlDown = false;
        }
        else if (keysym.sym == SDLK_LSHIFT)
        {
            leftShiftDown = false;
        }
        else if (keysym.sym == SDLK_RSHIFT)
        {
            rightShiftDown = false;
        }
    }

    void GameScene::onMouseDown(MouseButtonEvent event)
    {
        currentPanel->mouseDown(event);

        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            match(
                cursorMode.getValue(),
                [&](const AttackCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        if (hoveredUnit)
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                cursorMode.next(NormalCursorMode());
                            }
                        }
                        else
                        {
                            auto coord = getMouseTerrainCoordinate();
                            if (coord)
                            {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*coord));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*coord));
                                    cursorMode.next(NormalCursorMode());
                                }
                            }
                        }
                    }
                },
                [&](const MoveCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        auto coord = getMouseTerrainCoordinate();
                        if (coord)
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(*coord));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(selectedUnit, MoveOrder(*coord));
                                cursorMode.next(NormalCursorMode());
                            }
                        }
                    }
                },
                [&](const GuardCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        if (hoveredUnit && isFriendly(*hoveredUnit))
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(selectedUnit, GuardOrder(*hoveredUnit));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(selectedUnit, GuardOrder(*hoveredUnit));
                                cursorMode.next(NormalCursorMode());
                            }
                        }
                    }
                },
                [&](const BuildCursorMode& buildCursor) {
                    if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
                    {
                        if (hoverBuildInfo)
                        {
                            if (hoverBuildInfo->isValid)
                            {
                                auto topLeftWorld = simulation.terrain.heightmapIndexToWorldCorner(hoverBuildInfo->rect.x,
                                    hoverBuildInfo->rect.y);
                                auto x = topLeftWorld.x + ((SimScalar(hoverBuildInfo->rect.width) * MapTerrain::HeightTileWidthInWorldUnits) / 2_ss);
                                auto z = topLeftWorld.z + ((SimScalar(hoverBuildInfo->rect.height) * MapTerrain::HeightTileHeightInWorldUnits) / 2_ss);
                                auto y = simulation.terrain.getHeightAt(x, z);
                                SimVector buildPos(x, y, z);
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(*selectedUnit, BuildOrder(buildCursor.unitType, buildPos));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(*selectedUnit, BuildOrder(buildCursor.unitType, buildPos));
                                    cursorMode.next(NormalCursorMode());
                                }
                            }
                            else
                            {
                                if (sounds.notOkToBuild)
                                {
                                    playUiSound(*sounds.notOkToBuild);
                                }
                            }
                        }
                    }
                },
                [&](const NormalCursorMode&) {
                    if (isCursorOverMinimap())
                    {
                        if (leftClickMode())
                        {
                            for (const auto& selectedUnit : selectedUnits)
                            {
                                if (hoveredUnit)
                                {
                                    if (isEnemy(*hoveredUnit))
                                    {
                                        if (isShiftDown())
                                        {
                                            localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                        }
                                        else
                                        {
                                            localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                        }
                                    }
                                    else
                                    {
                                        if (getUnit(*hoveredUnit).isBeingBuilt())
                                        {
                                            if (isShiftDown())
                                            {
                                                localPlayerEnqueueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                            }
                                            else
                                            {
                                                localPlayerIssueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    auto coord = getMouseTerrainCoordinate();
                                    if (coord)
                                    {
                                        if (isShiftDown())
                                        {
                                            localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(*coord));
                                        }
                                        else
                                        {
                                            localPlayerIssueUnitOrder(selectedUnit, MoveOrder(*coord));
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            cursorMode.next(NormalCursorMode{NormalCursorMode::DraggingMinimapState()});
                        }
                    }
                    else if (isCursorOverWorld())
                    {
                        Point p(event.x, event.y);
                        auto worldViewportPos = sceneContext.viewport->toOtherViewport(worldViewport, p);
                        const auto cameraPosition = worldCameraState.getRoundedPosition();
                        Point originRelativePos(cameraPosition.x + worldViewportPos.x, cameraPosition.z + worldViewportPos.y);
                        cursorMode.next(NormalCursorMode{NormalCursorMode::SelectingState(sceneTime, originRelativePos)});
                    }
                });
        }
        else if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            match(
                cursorMode.getValue(),
                [&](const AttackCursorMode&) {
                    cursorMode.next(NormalCursorMode());
                },
                [&](const MoveCursorMode&) {
                    cursorMode.next(NormalCursorMode());
                },
                [&](const GuardCursorMode&) {
                    cursorMode.next(NormalCursorMode());
                },
                [&](const BuildCursorMode&) {
                    cursorMode.next(NormalCursorMode());
                },
                [&](const NormalCursorMode&) {
                    if (leftClickMode())
                    {
                        if (isCursorOverMinimap())
                        {
                            cursorMode.next(NormalCursorMode{NormalCursorMode::DraggingMinimapState()});
                        }
                        else if (isCursorOverWorld())
                        {
                            clearUnitSelection();
                        }
                    }
                    else
                    {
                        for (const auto& selectedUnit : selectedUnits)
                        {
                            if (hoveredUnit)
                            {
                                if (isEnemy(*hoveredUnit))
                                {
                                    if (isShiftDown())
                                    {
                                        localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                    }
                                    else
                                    {
                                        localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                    }
                                }
                                else
                                {
                                    if (getUnit(*hoveredUnit).isBeingBuilt())
                                    {
                                        if (isShiftDown())
                                        {
                                            localPlayerEnqueueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                        }
                                        else
                                        {
                                            localPlayerIssueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                        }
                                    }
                                }
                            }
                            else
                            {
                                auto coord = getMouseTerrainCoordinate();
                                if (coord)
                                {
                                    if (isShiftDown())
                                    {
                                        localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(*coord));
                                    }
                                    else
                                    {
                                        localPlayerIssueUnitOrder(selectedUnit, MoveOrder(*coord));
                                    }
                                }
                            }
                        }
                    }
                });
        }
    }

    void GameScene::onMouseUp(MouseButtonEvent event)
    {
        currentPanel->mouseUp(event);

        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            match(
                cursorMode.getValue(),
                [&](const NormalCursorMode& normalCursor) {
                    match(
                        normalCursor.state,
                        [&](const NormalCursorMode::SelectingState& state) {
                            Point p(event.x, event.y);
                            auto worldViewportPos = sceneContext.viewport->toOtherViewport(worldViewport, p);
                            const auto cameraPosition = worldCameraState.getRoundedPosition();
                            Point originRelativePos(cameraPosition.x + worldViewportPos.x, cameraPosition.z + worldViewportPos.y);

                            if (sceneTime - state.startTime < SceneTime(30) && state.startPosition.maxSingleDimensionDistance(originRelativePos) < 32)
                            {
                                if (hoveredUnit && getUnit(*hoveredUnit).isSelectableBy(localPlayerId))
                                {
                                    if (isShiftDown())
                                    {
                                        toggleUnitSelection(*hoveredUnit);
                                    }
                                    else
                                    {
                                        replaceUnitSelection(*hoveredUnit);
                                    }
                                }
                                else if (leftClickMode() && hoveredUnit)
                                {
                                    if (isEnemy(*hoveredUnit))
                                    {
                                        for (const auto& selectedUnit : selectedUnits)
                                        {
                                            if (isShiftDown())
                                            {
                                                localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                            }
                                            else
                                            {
                                                localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*hoveredUnit));
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (getUnit(*hoveredUnit).isBeingBuilt())
                                        {
                                            for (const auto& selectedUnit : selectedUnits)
                                            {
                                                if (isShiftDown())
                                                {
                                                    localPlayerEnqueueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                                }
                                                else
                                                {
                                                    localPlayerIssueUnitOrder(selectedUnit, CompleteBuildOrder(*hoveredUnit));
                                                }
                                            }
                                        }
                                    }
                                }
                                else if (leftClickMode())
                                {
                                    auto coord = getMouseTerrainCoordinate();
                                    if (coord)
                                    {
                                        for (const auto& selectedUnit : selectedUnits)
                                        {
                                            if (isShiftDown())
                                            {
                                                localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(*coord));
                                            }
                                            else
                                            {
                                                localPlayerIssueUnitOrder(selectedUnit, MoveOrder(*coord));
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (!isShiftDown())
                                    {
                                        clearUnitSelection();
                                    }
                                }
                            }
                            else
                            {
                                selectUnitsInBandbox(DiscreteRect::fromPoints(state.startPosition, originRelativePos));
                            }

                            cursorMode.next(NormalCursorMode{NormalCursorMode::UpState()});
                        },
                        [&](const NormalCursorMode::DraggingMinimapState&) {
                            cursorMode.next(NormalCursorMode{NormalCursorMode::UpState()});
                        },
                        [&](const NormalCursorMode::UpState&) {
                        });
                },
                [&](const auto&) {
                    // do nothing
                });
        }
        else if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            match(
                cursorMode.getValue(),
                [&](const NormalCursorMode& m) {
                    match(
                        m.state,
                        [&](const NormalCursorMode::DraggingMinimapState&) {
                            cursorMode.next(NormalCursorMode{NormalCursorMode::UpState()});
                        },
                        [](const auto&) {});
                },
                [](const auto&) {});
        }
    }

    void GameScene::onMouseMove(MouseMoveEvent event)
    {
        currentPanel->mouseMove(event);
    }

    void GameScene::onMouseWheel(MouseWheelEvent event)
    {
        currentPanel->mouseWheel(event);
    }

    Rectangle2f computeCameraConstraint(const MapTerrain& terrain, float viewportWidth, float viewportHeight)
    {
        auto cameraHalfWidth = viewportWidth / 2.0f;
        auto cameraHalfHeight = viewportHeight / 2.0f;

        auto top = simScalarToFloat(terrain.topInWorldUnits()) + cameraHalfHeight;
        auto left = simScalarToFloat(terrain.leftInWorldUnits()) + cameraHalfWidth;
        auto bottom = simScalarToFloat(terrain.bottomCutoffInWorldUnits()) - cameraHalfHeight;
        auto right = simScalarToFloat(terrain.rightCutoffInWorldUnits()) - cameraHalfWidth;

        if (left > right)
        {
            auto middle = (left + right) / 2.0f;
            left = middle;
            right = middle;
        }

        if (top > bottom)
        {
            auto middle = (top + bottom) / 2.0f;
            top = middle;
            bottom = middle;
        }

        return Rectangle2f::fromTLBR(top, left, bottom, right);
    }

    void GameScene::update(int millisecondsElapsed)
    {
        millisecondsBuffer += millisecondsElapsed;

        auto cameraConstraint = computeCameraConstraint(simulation.terrain, worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));

        // update camera position from keyboard arrows
        {
            const float speed = CameraPanSpeed * millisecondsElapsed / 1000.0f;
            int directionX = (right ? 1 : 0) - (left ? 1 : 0);
            int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

            if (directionX || directionZ)
            {
                trackingOn = false;

                auto dx = directionX * speed;
                auto dz = directionZ * speed;
                auto& cameraPos = worldCameraState.position;
                auto newPos = cameraConstraint.clamp(Vector2f(cameraPos.x + dx, cameraPos.z + dz));

                worldCameraState.position = Vector3f(newPos.x, cameraPos.y, newPos.y);
            }
        }

        // update camera position from edge scroll
        {
            const float speed = CameraPanSpeed * millisecondsElapsed / 1000.0f;

            auto mousePosition = getMousePosition();
            auto directionX = mousePosition.x == sceneContext.viewport->left()
                ? -1
                : mousePosition.x == sceneContext.viewport->right() - 1
                ? 1
                : 0;
            auto directionZ = mousePosition.y == sceneContext.viewport->top()
                ? -1
                : mousePosition.y == sceneContext.viewport->bottom() - 1
                ? 1
                : 0;

            if (directionX || directionZ)
            {
                trackingOn = false;

                auto dx = directionX * speed;
                auto dz = directionZ * speed;
                auto& cameraPos = worldCameraState.position;
                auto newPos = cameraConstraint.clamp(Vector2f(cameraPos.x + dx, cameraPos.z + dz));

                worldCameraState.position = Vector3f(newPos.x, cameraPos.y, newPos.y);
            }
        }

        // handle minimap dragging
        if (auto cursor = std::get_if<NormalCursorMode>(&cursorMode.getValue()); cursor != nullptr)
        {
            if (std::holds_alternative<NormalCursorMode::DraggingMinimapState>(cursor->state))
            {
                trackingOn = false;

                // ok, the cursor is dragging the minimap.
                // work out where the cursor is on the minimap,
                // convert that to the world, then set the camera's position to there
                // (clamped to map bounds)

                auto minimapToWorld = minimapToWorldMatrix(simulation.terrain, minimapRect);
                auto mousePos = getMousePosition();
                auto worldPos = minimapToWorld * Vector3f(static_cast<float>(mousePos.x) + 0.5f, static_cast<float>(mousePos.y) + 0.5, 0.0f);
                auto newCameraPos = cameraConstraint.clamp(Vector2f(worldPos.x, worldPos.z));
                worldCameraState.position = Vector3f(newCameraPos.x, worldCameraState.position.y, newCameraPos.y);
            }
        }

        // handle tracking
        {
            // TODO (kwh) - tracking of projectiles not yet implemented. E.g. while tracking Bertha or Nuke Silo,
            // screen should follow a projectile until it hits, then return to the tracking group

            if (trackingOn)
            {
                // get tracked unit position, or stop tracking if it's gone
                auto unit = tryGetUnit(trackedUnitId);
                if (unit && !unit->get().isDead())
                {
                    // Move camera... OTA behavior:
                    // For each x and z component (not euclidean distance), halve the distance from camera to unit each frame,
                    //  but limit to a max of 320 pixels per frame, at 30fps for normal speed (Scroll speed followed game speed, eg +10 scrolls faster).
                    // Presumably 320 to make scrolling look continuous on the lowest res setting of 640x480

                    // We will use millisecondsElapsed to interpolate for smoother scrolling at high fps in rwe,
                    // while maintaining similar scroll speed on the map; Speed is linear and clamped at 320 pixels/(1/30)s = 9600 pix/s,
                    const float maxScroll = 9.6f * millisecondsElapsed;
                    // To interpolate halving distance every 1/30s, we'll use the definition of geometric progression: a_n = a*r^(n); where:
                    //  a_n = distance (pixels) from the unit we should be after n 1/30s frames, a = current distance from the unit
                    //  r = common ratio i.e. 1/2, the ratio the distance should decrease every 1/30 seconds
                    //  n = # of OTA frames = seconds elapsed / (1/30 s per OTA frame) = (time_ms / 1000) * 30 = 3 * time_ms / 100

                    const auto& cameraPos = worldCameraState.position;
                    const auto unitPos = simVectorToFloat(unit->get().position);
                    const auto cameraPosDelta = unitPos - cameraPos;

                    float decayFactor = 1.0f - std::pow(.5f, .03f * millisecondsElapsed);

                    float newDelta_x = cameraPosDelta.x * decayFactor;
                    if (std::abs(newDelta_x) > maxScroll)
                    {
                        newDelta_x = newDelta_x < 0 ? -maxScroll : maxScroll;
                    }

                    float newDelta_z = cameraPosDelta.z * decayFactor;
                    if (std::abs(newDelta_z) > maxScroll)
                    {
                        newDelta_z = newDelta_z < 0 ? -maxScroll : maxScroll;
                    }

                    auto newPos = cameraConstraint.clamp(Vector2f(newDelta_x + cameraPos.x, newDelta_z + cameraPos.z));
                    worldCameraState.position = Vector3f(newPos.x, worldCameraState.position.y, newPos.y);
                }
            }
        }

        // reset cursor mode if shift is released and at least 1 order was queued since shift was held
        if (commandWasQueued && !isShiftDown())
        {
            cursorMode.next(NormalCursorMode());
            commandWasQueued = false;
        }

        hoveredUnit = getUnitUnderCursor();

        if (auto buildCursor = std::get_if<BuildCursorMode>(&cursorMode.getValue()); buildCursor != nullptr && isCursorOverWorld())
        {
            auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
            auto intersect = simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));

            if (intersect)
            {
                auto mc = unitFactory.getAdHocMovementClass(buildCursor->unitType);
                const auto& unitType = buildCursor->unitType;
                const auto& pos = *intersect;
                auto footprint = unitFactory.getUnitFootprint(unitType);
                auto footprintRect = computeFootprintRegion(pos, footprint.x, footprint.y);
                auto isValid = simulation.canBeBuiltAt(mc, footprintRect.x, footprintRect.y);
                hoverBuildInfo = HoverBuildInfo{footprintRect, isValid};
            }
            else
            {
                hoverBuildInfo = std::nullopt;
            }
        }
        else
        {
            hoverBuildInfo = std::nullopt;
        }

        if (!isCursorOverMinimap() && !isCursorOverWorld())
        {
            // The cursor is outside the world, so over UI elements.
            sceneContext.cursor->useCursor(CursorType::Normal);
        }
        else
        {
            match(
                cursorMode.getValue(),
                [&](const AttackCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Attack);
                },
                [&](const MoveCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Move);
                },
                [&](const GuardCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Guard);
                },
                [&](const BuildCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Normal);
                },
                [&](const NormalCursorMode&) {
                    if (leftClickMode())
                    {
                        if (hoveredUnit && getUnit(*hoveredUnit).isSelectableBy(localPlayerId))
                        {
                            sceneContext.cursor->useCursor(CursorType::Select);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).canAttack; }) && hoveredUnit && isEnemy(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Attack);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).builder; }) && hoveredUnit && getUnit(*hoveredUnit).isBeingBuilt())
                        {
                            sceneContext.cursor->useCursor(CursorType::Repair);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).canMove; }))
                        {
                            sceneContext.cursor->useCursor(CursorType::Move);
                        }
                        else
                        {
                            sceneContext.cursor->useCursor(CursorType::Normal);
                        }
                    }
                    else
                    {
                        if (hoveredUnit && getUnit(*hoveredUnit).isSelectableBy(localPlayerId))
                        {
                            sceneContext.cursor->useCursor(CursorType::Select);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).canAttack; }) && hoveredUnit && isEnemy(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Red);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).builder; }) && hoveredUnit && isFriendly(*hoveredUnit) && getUnit(*hoveredUnit).isBeingBuilt())
                        {
                            sceneContext.cursor->useCursor(CursorType::Green);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return getUnit(id).canGuard; }) && hoveredUnit && isFriendly(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Green);
                        }
                        else
                        {
                            sceneContext.cursor->useCursor(CursorType::Normal);
                        }
                    }
                });
        }

        auto maxRtt = std::clamp(gameNetworkService->getMaxAverageRttMillis(), 16.0f, 2000.0f);
        auto highCommandLatencyMillis = maxRtt + (maxRtt / 4.0f) + 200.0f;
        auto commandLatencyFrames = static_cast<unsigned int>(highCommandLatencyMillis / 16.0f) + 1;
        auto targetCommandBufferSize = commandLatencyFrames;

        auto bufferedCommandCount = playerCommandService->bufferedCommandCount(localPlayerId);

        spdlog::get("rwe")->debug("Buffer levels (real/target) {0}/{1}", bufferedCommandCount, targetCommandBufferSize);

        // If we have too many commands buffered,
        // defer submitting commands this frame
        // so that we drop back down to the threshold.
        if (bufferedCommandCount <= targetCommandBufferSize)
        {
            // Queue up commands collected from the local player
            playerCommandService->pushCommands(localPlayerId, localPlayerCommandBuffer);
            gameNetworkService->submitCommands(sceneTime, localPlayerCommandBuffer);
            localPlayerCommandBuffer.clear();
            ++bufferedCommandCount;
        }

        // fill up to the required threshold
        for (; bufferedCommandCount < targetCommandBufferSize; ++bufferedCommandCount)
        {
            playerCommandService->pushCommands(localPlayerId, std::vector<PlayerCommand>());
            gameNetworkService->submitCommands(sceneTime, std::vector<PlayerCommand>());
        }

        // Queue up commands from the computer players
        for (Index i = 0; i < getSize(simulation.players); ++i)
        {
            PlayerId id(i);
            const auto& player = simulation.players[i];
            if (player.type == GamePlayerType::Computer)
            {
                if (playerCommandService->bufferedCommandCount(id) == 0)
                {
                    // TODO: implement computer AI logic to decide commands here
                    playerCommandService->pushCommands(id, std::vector<PlayerCommand>());
                }
            }
        }

        // If we are waiting to swap in a new unit GUI panel, do that now
        if (nextPanel)
        {
            currentPanel = std::move(*nextPanel);
            nextPanel = std::nullopt;
            attachOrdersMenuEventHandlers();
        }

        auto averageSceneTime = gameNetworkService->estimateAvergeSceneTime(sceneTime);

        // allow skipping sim frames every so often to get back down to average.
        // We tolerate X frames of drift in either direction to cope with noisiness in the estimation.
        const SceneTime frameTolerance(3);
        const SceneTime frameCheckInterval(5);
        auto highSceneTime = averageSceneTime + frameTolerance;
        auto lowSceneTime = averageSceneTime <= frameTolerance ? SceneTime{0} : averageSceneTime - frameTolerance;
        for (; millisecondsBuffer >= SimMillisecondsPerTick; millisecondsBuffer -= SimMillisecondsPerTick)
        {
            if (sceneTime % frameCheckInterval != SceneTime(0) || sceneTime <= highSceneTime)
            {
                tryTickGame();

                // simulate an extra frame to catch up every so often
                if (sceneTime % frameCheckInterval == SceneTime(0) && sceneTime < lowSceneTime)
                {
                    tryTickGame();
                }
            }
        }

        renderDebugWindow();
    }

    std::optional<UnitId> GameScene::spawnUnit(const std::string& unitType, PlayerId owner, const SimVector& position, std::optional<const std::reference_wrapper<SimAngle>> rotation)
    {
        auto unitFbi = unitDatabase.getUnitInfo(unitType);
        auto unit = unitFactory.createUnit(unitType, owner, position, rotation);
        if (unit.floater || unit.canHover)
        {
            unit.position.y = rweMax(simulation.terrain.getSeaLevel(), unit.position.y);
            unit.previousPosition.y = unit.position.y;
        }

        // TODO: if we failed to add the unit throw some warning
        auto unitId = simulation.tryAddUnit(std::move(unit));

        if (unitId)
        {
            unitBehaviorService.onCreate(*unitId);

            // initialise local-player-specific UI data
            const auto& unit = getUnit(*unitId);
            unitGuiInfos.insert_or_assign(*unitId, UnitGuiInfo{unit.builder ? UnitGuiInfo::Section::Build : UnitGuiInfo::Section::Orders, 0});
        }

        return unitId;
    }

    std::optional<std::reference_wrapper<Unit>> GameScene::spawnCompletedUnit(const std::string& unitType, PlayerId owner, const SimVector& position)
    {
        auto unitId = spawnUnit(unitType, owner, position, std::nullopt);
        if (unitId)
        {
            auto& unit = getUnit(*unitId);
            // units start as unbuilt nanoframes,
            // we we need to convert it immediately into a completed unit.
            unit.finishBuilding();

            return unit;
        }

        return std::nullopt;
    }

    MapFeature createFeature(TextureService& textureService, const SimVector& pos, const FeatureTdf& definition)
    {
        MapFeature f;
        f.footprintX = definition.footprintX;
        f.footprintZ = definition.footprintZ;
        f.height = SimScalar(definition.height);
        f.isBlocking = definition.blocking;
        f.isIndestructible = definition.indestructible;
        f.metal = definition.metal;
        f.position = pos;

        if (!definition.object.empty())
        {
            f.renderInfo = FeatureObjectInfo{definition.object};
        }
        else
        {
            FeatureSpriteInfo spriteInfo;
            spriteInfo.transparentAnimation = definition.animTrans;
            spriteInfo.transparentShadow = definition.shadTrans;
            if (!definition.fileName.empty() && !definition.seqName.empty())
            {
                spriteInfo.animation = textureService.getGafEntry("anims/" + definition.fileName + ".GAF", definition.seqName);
            }
            if (!spriteInfo.animation)
            {
                spriteInfo.animation = textureService.getDefaultSpriteSeries();
            }

            if (!definition.fileName.empty() && !definition.seqNameShad.empty())
            {
                // Some third-party features have broken shadow anim names (e.g. "empty"),
                // ignore them if they don't exist.
                spriteInfo.shadowAnimation = textureService.tryGetGafEntry("anims/" + definition.fileName + ".GAF", definition.seqNameShad);
            }
            f.renderInfo = std::move(spriteInfo);
        }


        return f;
    }

    void GameScene::trySpawnFeature(const std::string& featureType, const SimVector& position, SimAngle rotation)
    {
        const auto& featureDefinition = featuresMap.at(featureType);
        auto feature = createFeature(*sceneContext.textureService, position, featureDefinition);

        // FIXME: simulation needs to support failing to spawn in a feature
        simulation.addFeature(std::move(feature));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        worldCameraState.position = newPosition;
    }

    const MapTerrain& GameScene::getTerrain() const
    {
        return simulation.terrain;
    }

    void GameScene::showObject(UnitId unitId, const std::string& name)
    {
        simulation.showObject(unitId, name);
    }

    void GameScene::hideObject(UnitId unitId, const std::string& name)
    {
        simulation.hideObject(unitId, name);
    }

    void
    GameScene::moveObject(UnitId unitId, const std::string& name, Axis axis, SimScalar position, SimScalar speed)
    {
        simulation.moveObject(unitId, name, axis, position, speed);
    }

    void GameScene::moveObjectNow(UnitId unitId, const std::string& name, Axis axis, SimScalar position)
    {
        simulation.moveObjectNow(unitId, name, axis, position);
    }

    void GameScene::turnObject(UnitId unitId, const std::string& name, Axis axis, SimAngle angle, SimScalar speed)
    {
        simulation.turnObject(unitId, name, axis, angle, speed);
    }

    void GameScene::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, SimAngle angle)
    {
        simulation.turnObjectNow(unitId, name, axis, angle);
    }

    bool GameScene::isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const
    {
        return simulation.isPieceMoving(unitId, name, axis);
    }

    bool GameScene::isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const
    {
        return simulation.isPieceTurning(unitId, name, axis);
    }

    Matrix4x<SimScalar> GameScene::getUnitPieceLocalTransform(UnitId unitId, const std::string& pieceName) const
    {
        const auto& unit = getUnit(unitId);
        const auto& modelDef = unitDatabase.getUnitModelDefinition(unit.objectName).value().get();
        return getPieceTransform(pieceName, modelDef, unit.pieces);
    }

    SimVector GameScene::getUnitPiecePosition(UnitId unitId, const std::string& pieceName) const
    {
        const auto& unit = getUnit(unitId);
        const auto& modelDef = unitDatabase.getUnitModelDefinition(unit.objectName).value().get();
        auto pieceTransform = getPieceTransform(pieceName, modelDef, unit.pieces);
        return unit.getTransform() * pieceTransform * SimVector(0_ss, 0_ss, 0_ss);
    }

    GameTime
    GameScene::getGameTime() const
    {
        return simulation.gameTime;
    }

    void GameScene::playUiSound(const AudioService::SoundHandle& handle)
    {
        sceneContext.audioService->playSoundIfFree(handle, UnitSelectChannel);
    }

    void GameScene::playNotificationSound(const PlayerId& playerId, const AudioService::SoundHandle& sound)
    {
        if (playerId == localPlayerId)
        {
            sceneContext.audioService->playSoundIfFree(sound, UnitSelectChannel);
        }
    }


    std::optional<std::string> getSoundName(const SoundClass& c, UnitSoundType sound)
    {
        switch (sound)
        {
            case UnitSoundType::Select1:
                return c.select1;
            case UnitSoundType::UnitComplete:
                return c.unitComplete;
            case UnitSoundType::Activate:
                return c.activate;
            case UnitSoundType::Deactivate:
                return c.deactivate;
            case UnitSoundType::Ok1:
                return c.ok1;
            case UnitSoundType::Arrived1:
                return c.arrived1;
            case UnitSoundType::Cant1:
                return c.cant1;
            case UnitSoundType::UnderAttack:
                return c.underAttack;
            case UnitSoundType::Build:
                return c.build;
            case UnitSoundType::Repair:
                return c.repair;
            case UnitSoundType::Working:
                return c.working;
            case UnitSoundType::Cloak:
                return c.cloak;
            case UnitSoundType::Uncloak:
                return c.uncloak;
            case UnitSoundType::Capture:
                return c.capture;
            case UnitSoundType::Count5:
                return c.count5;
            case UnitSoundType::Count4:
                return c.count4;
            case UnitSoundType::Count3:
                return c.count3;
            case UnitSoundType::Count2:
                return c.count2;
            case UnitSoundType::Count1:
                return c.count1;
            case UnitSoundType::Count0:
                return c.count0;
            case UnitSoundType::CancelDestruct:
                return c.cancelDestruct;
            default:
                throw std::logic_error("Invalid sound type");
        }
    }

    std::optional<AudioService::SoundHandle> getSound(const UnitDatabase& db, const MeshDatabase& meshDb, const std::string& unitType, UnitSoundType soundType)
    {
        const auto& fbi = db.getUnitInfo(unitType);
        const auto& soundClass = meshDb.getSoundClassOrDefault(fbi.soundCategory);
        const auto& soundId = getSoundName(soundClass, soundType);
        if (soundId)
        {
            return meshDb.tryGetSoundHandle(*soundId);
        }
        return std::nullopt;
    }

    void GameScene::playUnitNotificationSound(const PlayerId& playerId, const std::string& unitType, UnitSoundType soundType)
    {
        auto sound = getSound(unitDatabase, meshDatabase, unitType, soundType);
        if (sound)
        {
            playNotificationSound(playerId, *sound);
        }
    }

    void GameScene::playSoundAt(const Vector3f& /*position*/, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a position-aware channel
        auto channel = sceneContext.audioService->playSound(sound);
        std::scoped_lock<std::mutex> lock(playingUnitChannelsLock);
        playingUnitChannels.insert(channel);
        sceneContext.audioService->setVolume(channel, computeSoundVolume(playingUnitChannels.size()));
    }

    void GameScene::playWeaponStartSound(const Vector3f& position, const std::string& weaponType)
    {

        const auto& weaponMediaInfo = meshDatabase.getWeapon(weaponType);
        if (weaponMediaInfo.soundStart)
        {
            auto sound = meshDatabase.tryGetSoundHandle(*weaponMediaInfo.soundStart);
            if (sound)
            {
                playSoundAt(position, *sound);
            }
        }
    }

    void GameScene::playWeaponImpactSound(const Vector3f& position, const std::string& weaponType, ImpactType impactType)
    {
        const auto& weaponMediaInfo = meshDatabase.getWeapon(weaponType);
        switch (impactType)
        {
            case ImpactType::Normal:
            {
                if (weaponMediaInfo.soundHit)
                {
                    auto sound = meshDatabase.tryGetSoundHandle(*weaponMediaInfo.soundHit);
                    if (sound)
                    {
                        playSoundAt(position, *sound);
                    }
                }
                break;
            }
            case ImpactType::Water:
            {
                if (weaponMediaInfo.soundWater)
                {
                    auto sound = meshDatabase.tryGetSoundHandle(*weaponMediaInfo.soundWater);
                    if (sound)
                    {
                        playSoundAt(position, *sound);
                    }
                }
                break;
            }
        }
    }

    void GameScene::spawnWeaponImpactExplosion(const Vector3f& position, const std::string& weaponType, ImpactType impactType)
    {
        const auto& weaponMediaInfo = meshDatabase.getWeapon(weaponType);

        switch (impactType)
        {
            case ImpactType::Normal:
            {
                if (weaponMediaInfo.explosionAnim)
                {
                    spawnExplosion(position, *weaponMediaInfo.explosionAnim);
                }
                if (weaponMediaInfo.endSmoke)
                {
                    createLightSmoke(position);
                }
                break;
            }
            case ImpactType::Water:
            {
                if (weaponMediaInfo.waterExplosionAnim)
                {
                    spawnExplosion(position, *weaponMediaInfo.waterExplosionAnim);
                }
                break;
            }
        }

        spawnFlash(position);
    }

    void GameScene::onChannelFinished(int channel)
    {
        std::scoped_lock<std::mutex> lock(playingUnitChannelsLock);
        playingUnitChannels.erase(channel);
    }

    Matrix4f GameScene::worldToMinimapMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect)
    {
        auto view = Matrix4f::rotationToAxes(
            Vector3f(1.0f, 0.0f, 0.0f),
            Vector3f(0.0f, 0.0f, 1.0f),
            Vector3f(0.0f, -1.0f, 0.0f));
        auto cabinet = Matrix4f::cabinetProjection(0.0f, 0.5f);
        auto orthographic = Matrix4f::orthographicProjection(
            simScalarToFloat(terrain.leftInWorldUnits()),
            simScalarToFloat(terrain.rightCutoffInWorldUnits()),
            simScalarToFloat(terrain.bottomCutoffInWorldUnits()),
            simScalarToFloat(terrain.topInWorldUnits()),
            -1000.0f,
            1000.0f);
        auto worldProjection = orthographic * cabinet;
        auto minimapInverseProjection = Matrix4f::inverseOrthographicProjection(
            minimapRect.left(),
            minimapRect.right(),
            minimapRect.bottom(),
            minimapRect.top(),
            -1.0f,
            1.0f);
        return minimapInverseProjection * worldProjection * view;
    }

    Matrix4f GameScene::minimapToWorldMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect)
    {
        auto view = Matrix4f::rotationToAxes(
            Vector3f(1.0f, 0.0f, 0.0f),
            Vector3f(0.0f, 0.0f, 1.0f),
            Vector3f(0.0f, -1.0f, 0.0f));
        auto inverseView = view.transposed();
        auto inverseCabinet = Matrix4f::cabinetProjection(0.0f, -0.5f);
        auto inverseOrthographic = Matrix4f::inverseOrthographicProjection(
            simScalarToFloat(terrain.leftInWorldUnits()),
            simScalarToFloat(terrain.rightCutoffInWorldUnits()),
            simScalarToFloat(terrain.bottomCutoffInWorldUnits()),
            simScalarToFloat(terrain.topInWorldUnits()),
            -1000.0f,
            1000.0f);
        auto worldInverseProjection = inverseCabinet * inverseOrthographic;
        auto minimapProjection = Matrix4f::orthographicProjection(
            minimapRect.left(),
            minimapRect.right(),
            minimapRect.bottom(),
            minimapRect.top(),
            -1.0f,
            1.0f);
        return inverseView * worldInverseProjection * minimapProjection;
    }

    void GameScene::tryTickGame()
    {
        if (!playerCommandService->checkHashes())
        {
            std::ofstream dumpFile;
            dumpFile.open("rwe-dump-" + std::to_string(std::rand()) + ".json");
            dumpFile << dumpJson(simulation);
            dumpFile.close();
            throw std::runtime_error("Desync detected");
        }

        auto playerCommands = playerCommandService->tryPopCommands();
        if (!playerCommands)
        {
            spdlog::get("rwe")->error("Blocked waiting for player commands");
            return;
        }

        sceneTime += SceneTime(1);
        simulation.gameTime += GameTime(1);

        processActions();

        processPlayerCommands(*playerCommands);

        // run resource updates once per second
        if (simulation.gameTime % GameTime(SimTicksPerSecond) == GameTime(0))
        {
            // recalculate max energy and metal storage
            for (auto& player : simulation.players)
            {
                player.maxEnergy = Energy(0);
                player.maxMetal = Metal(0);
            }

            for (auto& entry : simulation.units)
            {
                auto& unit = entry.second;
                if (!unit.isBeingBuilt())
                {
                    getPlayer(unit.owner).maxMetal += unit.metalStorage;
                    getPlayer(unit.owner).maxEnergy += unit.energyStorage;
                }
            }

            for (auto& player : simulation.players)
            {
                player.metal += player.metalProductionBuffer;
                player.metalProductionBuffer = Metal(0);
                player.energy += player.energyProductionBuffer;
                player.energyProductionBuffer = Energy(0);

                if (player.metal > Metal(0))
                {
                    player.metal -= player.actualMetalConsumptionBuffer;
                    player.actualMetalConsumptionBuffer = Metal(0);
                    player.metalStalled = false;
                }
                else
                {
                    player.metalStalled = true;
                }

                player.previousDesiredMetalConsumptionBuffer = player.desiredMetalConsumptionBuffer;
                player.desiredMetalConsumptionBuffer = Metal(0);

                if (player.energy > Energy(0))
                {
                    player.energy -= player.actualEnergyConsumptionBuffer;
                    player.actualEnergyConsumptionBuffer = Energy(0);
                    player.energyStalled = false;
                }
                else
                {
                    player.energyStalled = true;
                }

                player.previousDesiredEnergyConsumptionBuffer = player.desiredEnergyConsumptionBuffer;
                player.desiredEnergyConsumptionBuffer = Energy(0);

                if (player.metal > player.maxMetal)
                {
                    player.metal = player.maxMetal;
                }

                if (player.energy > player.maxEnergy)
                {
                    player.energy = player.maxEnergy;
                }
            }

            for (auto& entry : simulation.units)
            {
                const auto& unitId = entry.first;
                auto& unit = entry.second;

                unit.resetResourceBuffers();

                if (!unit.isBeingBuilt())
                {
                    simulation.addResourceDelta(unitId, unit.energyMake, unit.metalMake);
                }

                if (unit.activated)
                {
                    if (unit.isSufficientlyPowered)
                    {
                        // extract metal
                        if (unit.extractsMetal != Metal(0))
                        {
                            auto footprint = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                            auto metalValue = simulation.metalGrid.accumulate(simulation.metalGrid.clipRegion(footprint), 0u, std::plus<>());
                            simulation.addResourceDelta(unitId, Energy(0), Metal(metalValue * unit.extractsMetal.value));
                        }
                    }

                    unit.isSufficientlyPowered = simulation.addResourceDelta(unitId, -unit.energyUse, -unit.metalUse);
                }
            }
        }

        pathFindingService.update();

        // run unit scripts
        for (auto& entry : simulation.units)
        {
            auto unitId = entry.first;
            auto& unit = entry.second;

            unitBehaviorService.update(unitId);

            for (auto& piece : unit.pieces)
            {
                piece.update(SimScalar(SimMillisecondsPerTick) / 1000_ss);
            }

            cobExecutionService.run(*this, simulation, unitId);
        }

        updateProjectiles();

        updateFlashes();

        updateExplosions(meshDatabase, simulation.gameTime, explosions);

        // if a commander died this frame, kill the player that owns it
        for (const auto& p : simulation.units)
        {
            if (p.second.isCommander() && p.second.isDead())
            {
                killPlayer(p.second.owner);
            }
        }

        auto winStatus = simulation.computeWinStatus();
        match(
            winStatus,
            [&](const WinStatusWon&) {
                delay(SceneTime(5 * 30), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
            },
            [&](const WinStatusDraw&) {
                delay(SceneTime(5 * 30), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
            },
            [&](const WinStatusUndecided&) {
                // do nothing, game still in progress
            });

        deleteDeadUnits();

        deleteDeadProjectiles();

        spawnNewUnits();

        auto gameHash = simulation.computeHash();
        playerCommandService->pushHash(localPlayerId, gameHash);
        gameNetworkService->submitGameHash(gameHash);

        if (stateLogStream)
        {
            *stateLogStream << dumpJson(simulation) << std::endl;
        }
    }

    std::optional<UnitId> GameScene::getUnitUnderCursor() const
    {
        if (isCursorOverMinimap())
        {
            auto mousePos = getMousePosition();

            auto worldToMinimap = worldToMinimapMatrix(simulation.terrain, minimapRect);

            for (const auto& [unitId, unit] : simulation.units)
            {
                // convert to minimap rect
                auto minimapPos = worldToMinimap * simVectorToFloat(unit.position);
                minimapPos.x = std::floor(minimapPos.x);
                minimapPos.y = std::floor(minimapPos.y);
                auto ownerId = unit.owner;
                auto colorIndex = getPlayer(ownerId).color;
                const auto& sprite = *minimapDots->sprites[colorIndex.value];
                auto bounds = sprite.bounds;

                // test cursor against the rect
                Vector2f mousePosFloat(static_cast<float>(mousePos.x) + 0.5f, static_cast<float>(mousePos.y) + 0.5f);
                if (bounds.contains(mousePosFloat - minimapPos.xy()))
                {
                    return unitId;
                }
            }

            return std::nullopt;
        }

        if (isCursorOverWorld())
        {
            auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
            return getFirstCollidingUnit(ray);
        }

        return std::nullopt;
    }

    Vector2f GameScene::screenToWorldClipSpace(Point p) const
    {
        return worldViewport.toClipSpace(sceneContext.viewport->toOtherViewport(worldViewport, p));
    }

    bool GameScene::isCursorOverMinimap() const
    {
        auto mousePos = getMousePosition();
        return minimapRect.contains(mousePos.x, mousePos.y);
    }

    bool GameScene::isCursorOverWorld() const
    {
        return worldViewport.contains(getMousePosition());
    }

    Point GameScene::getMousePosition() const
    {
        int x;
        int y;
        sceneContext.sdl->getMouseState(&x, &y);
        return Point(x, y);
    }

    std::optional<UnitId> GameScene::getFirstCollidingUnit(const Ray3f& ray) const
    {
        auto bestDistance = std::numeric_limits<float>::infinity();
        std::optional<UnitId> it;

        for (const auto& entry : simulation.units)
        {
            const auto& fbi = unitDatabase.getUnitInfo(entry.second.unitType);
            auto selectionMesh = meshDatabase.getSelectionCollisionMesh(fbi.objectName);
            auto distance = selectionIntersect(entry.second, *selectionMesh.value(), ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it = entry.first;
            }
        }

        return it;
    }

    std::optional<float> GameScene::selectionIntersect(const Unit& unit, const CollisionMesh& mesh, const Ray3f& ray) const
    {
        auto inverseTransform = toFloatMatrix(unit.getInverseTransform());
        auto line = ray.toLine();
        Line3f modelSpaceLine(inverseTransform * line.start, inverseTransform * line.end);
        auto v = mesh.intersectLine(modelSpaceLine);
        if (!v)
        {
            return std::nullopt;
        }

        return ray.origin.distance(*v);
    }

    std::optional<SimVector> GameScene::getMouseTerrainCoordinate() const
    {
        if (isCursorOverMinimap())
        {
            auto transform = minimapToWorldMatrix(simulation.terrain, minimapRect);
            auto mousePos = getMousePosition();
            auto mouseX = static_cast<float>(mousePos.x) + 0.5f;
            auto mouseY = static_cast<float>(mousePos.y) + 0.5f;

            auto startPoint = transform * Vector3f(mouseX, mouseY, -1.0f);
            auto endPoint = transform * Vector3f(mouseX, mouseY, 1.0f);
            auto direction = endPoint - startPoint;
            Ray3f ray(startPoint, direction);
            return simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));
        }

        if (isCursorOverWorld())
        {
            auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
            return simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));
        }

        return std::nullopt;
    }

    void GameScene::localPlayerIssueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto kind = PlayerUnitCommand::IssueOrder::IssueKind::Immediate;
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::IssueOrder(order, kind)));

        if (std::holds_alternative<BuildOrder>(order))
        {
            if (sounds.okToBuild)
            {
                playUiSound(*sounds.okToBuild);
            }
        }
        else
        {
            const auto& unit = getUnit(unitId);
            auto handle = getSound(unitDatabase, meshDatabase, unit.unitType, UnitSoundType::Ok1);
            if (handle)
            {
                playUiSound(*handle);
            }
        }
    }

    void GameScene::localPlayerEnqueueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto kind = PlayerUnitCommand::IssueOrder::IssueKind::Queued;
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::IssueOrder(order, kind)));

        if (std::holds_alternative<BuildOrder>(order))
        {
            if (sounds.okToBuild)
            {
                playUiSound(*sounds.okToBuild);
            }
        }

        commandWasQueued = true;
    }

    void GameScene::localPlayerStopUnit(UnitId unitId)
    {
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::Stop()));

        const auto& unit = getUnit(unitId);
        auto handle = getSound(unitDatabase, meshDatabase, unit.unitType, UnitSoundType::Ok1);
        if (handle)
        {
            playUiSound(*handle);
        }
    }

    void GameScene::localPlayerSetFireOrders(UnitId unitId, UnitFireOrders orders)
    {
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::SetFireOrders{orders}));
    }

    void GameScene::localPlayerSetOnOff(UnitId unitId, bool on)
    {
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::SetOnOff{on}));
    }

    void GameScene::localPlayerModifyBuildQueue(UnitId unitId, const std::string& unitType, int count)
    {
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::ModifyBuildQueue{count, unitType}));

        updateUnconfirmedBuildQueueDelta(unitId, unitType, count);
        refreshBuildGuiTotal(unitId, unitType);
    }

    void GameScene::issueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().clearOrders();
            unit->get().addOrder(order);
        }
    }

    void GameScene::enqueueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().addOrder(order);
        }
    }

    void GameScene::stopUnit(UnitId unitId)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().clearOrders();
        }
    }

    void GameScene::setFireOrders(UnitId unitId, UnitFireOrders orders)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().fireOrders = orders;

            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && *selectedUnit == unitId)
            {
                fireOrders.next(orders);
            }
        }
    }

    void GameScene::startTrack()
    {
        if (selectedUnits.size())
        {
            // sort selection by unit id so repeated 'T' keydown cycles through all units in a group consistently
            std::vector<UnitId> unitIds;
            for (const auto& u : selectedUnits)
            {
                unitIds.push_back(u);
            }
            std::sort(unitIds.begin(), unitIds.end());

            // If already tracking, check if currently tracked unit is in this selection. If it is, select the next id in the group.
            auto it = std::find(unitIds.begin(), unitIds.end(), trackedUnitId);
            if (trackingOn && it != unitIds.end() && ++it != unitIds.end())
            {
                trackedUnitId = *it;
            }
            else
            {
                trackedUnitId = unitIds[0];
            }
            trackingOn = true;
        }
        else
        {
            trackingOn = false;
        }
    }

    bool GameScene::isCtrlDown() const
    {
        return leftCtrlDown || rightCtrlDown;
    }

    bool GameScene::isShiftDown() const
    {
        return leftShiftDown || rightShiftDown;
    }

    void GameScene::handleEscapeDown()
    {
        match(
            cursorMode.getValue(),
            [this](const NormalCursorMode&) {
                clearUnitSelection();
            },
            [this](const auto&) {
                cursorMode.next(NormalCursorMode());
            });
    }

    Unit& GameScene::getUnit(UnitId id)
    {
        return simulation.getUnit(id);
    }

    const Unit& GameScene::getUnit(UnitId id) const
    {
        return simulation.getUnit(id);
    }

    std::optional<std::reference_wrapper<Unit>> GameScene::tryGetUnit(UnitId id)
    {
        return simulation.tryGetUnit(id);
    }

    std::optional<std::reference_wrapper<const Unit>> GameScene::tryGetUnit(UnitId id) const
    {
        return simulation.tryGetUnit(id);
    }

    GamePlayerInfo& GameScene::getPlayer(PlayerId player)
    {
        return simulation.getPlayer(player);
    }

    const GamePlayerInfo& GameScene::getPlayer(PlayerId player) const
    {
        return simulation.getPlayer(player);
    }

    DiscreteRect
    GameScene::computeFootprintRegion(const SimVector& position, unsigned int footprintX, unsigned int footprintZ) const
    {
        return simulation.computeFootprintRegion(position, footprintX, footprintZ);
    }

    bool GameScene::isCollisionAt(const DiscreteRect& rect, UnitId self) const
    {
        return simulation.isCollisionAt(rect, self);
    }

    void GameScene::moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId)
    {
        simulation.moveUnitOccupiedArea(oldRect, newRect, unitId);
    }

    GameSimulation& GameScene::getSimulation()
    {
        return simulation;
    }

    const GameSimulation& GameScene::getSimulation() const
    {
        return simulation;
    }

    const UnitDatabase& GameScene::getUnitDatabase() const
    {
        return unitDatabase;
    }

    const MeshDatabase& GameScene::getMeshDatabase() const
    {
        return meshDatabase;
    }

    bool GameScene::isEnemy(UnitId id) const
    {
        // TODO: consider allies/teams here
        return !getUnit(id).isOwnedBy(localPlayerId);
    }

    bool GameScene::isFriendly(UnitId id) const
    {
        return !isEnemy(id);
    }

    void GameScene::updateProjectiles()
    {
        auto gameTime = getGameTime();
        for (auto& [id, projectile] : simulation.projectiles)
        {
            const auto& weaponMediaInfo = meshDatabase.getWeapon(projectile.weaponType);

            // remove if it's time to die
            if (projectile.dieOnFrame && *projectile.dieOnFrame <= gameTime)
            {
                projectile.isDead = true;
                if (weaponMediaInfo.endSmoke)
                {
                    createLightSmoke(simVectorToFloat(projectile.position));
                }
                continue;
            }

            if (projectile.gravity)
            {
                projectile.velocity.y -= 112_ss / (30_ss * 30_ss);
            }
            projectile.previousPosition = projectile.position;
            projectile.position += projectile.velocity;

            // emit smoke trail
            if (weaponMediaInfo.smokeTrail)
            {
                if (gameTime > projectile.lastSmoke + *weaponMediaInfo.smokeTrail)
                {
                    createLightSmoke(simVectorToFloat(projectile.position));
                    projectile.lastSmoke = gameTime;
                }
            }

            // test collision with terrain
            auto terrainHeight = simulation.terrain.tryGetHeightAt(projectile.position.x, projectile.position.z);
            if (!terrainHeight)
            {
                // silently remove projectiles that go outside the map
                projectile.isDead = true;
                continue;
            }

            auto seaLevel = simulation.terrain.getSeaLevel();

            // test collision with sea
            // FIXME: waterweapons should be allowed in water
            if (seaLevel > *terrainHeight && projectile.position.y <= seaLevel)
            {
                doProjectileImpact(projectile, ImpactType::Water);
                projectile.isDead = true;
            }
            else if (projectile.position.y <= *terrainHeight)
            {
                if (projectile.groundBounce)
                {
                    projectile.velocity.y = 0_ss;
                    projectile.position.y = projectile.previousPosition.y;
                }
                else
                {
                    doProjectileImpact(projectile, ImpactType::Normal);
                    projectile.isDead = true;
                }
            }
            else
            {
                // detect collision with something's footprint
                auto heightMapPos = simulation.terrain.worldToHeightmapCoordinate(projectile.position);
                auto cellValue = simulation.occupiedGrid.tryGet(heightMapPos);
                if (cellValue)
                {
                    auto collides = projectileCollides(simulation, projectile, cellValue->get());
                    if (collides)
                    {
                        doProjectileImpact(projectile, ImpactType::Normal);
                        projectile.isDead = true;
                    }
                }
            }
        }
    }

    void GameScene::updateFlashes()
    {
        flashes.erase(
            std::remove_if(
                flashes.begin(),
                flashes.end(),
                [&](const auto& flash) { return flash.isFinished(simulation.gameTime); }),
            flashes.end());
    }

    void GameScene::doProjectileImpact(const Projectile& projectile, ImpactType impactType)
    {
        playWeaponImpactSound(simVectorToFloat(projectile.position), projectile.weaponType, impactType);
        spawnWeaponImpactExplosion(simVectorToFloat(projectile.position), projectile.weaponType, impactType);

        applyDamageInRadius(projectile.position, projectile.damageRadius, projectile);
    }

    void GameScene::applyDamageInRadius(const SimVector& position, SimScalar radius, const Projectile& projectile)
    {
        auto minX = position.x - radius;
        auto maxX = position.x + radius;
        auto minZ = position.z - radius;
        auto maxZ = position.z + radius;

        auto minPoint = simulation.terrain.worldToHeightmapCoordinate(SimVector(minX, position.y, minZ));
        auto maxPoint = simulation.terrain.worldToHeightmapCoordinate(SimVector(maxX, position.y, maxZ));
        auto minCell = simulation.occupiedGrid.clampToCoords(minPoint);
        auto maxCell = simulation.occupiedGrid.clampToCoords(maxPoint);

        assert(minCell.x <= maxCell.x);
        assert(minCell.y <= maxCell.y);

        auto radiusSquared = radius * radius;

        std::unordered_set<UnitId> seenUnits;

        auto region = GridRegion::fromCoordinates(minCell, maxCell);

        // for each cell
        region.forEach([&](const auto& coords) {
            // check if it's in range
            auto cellCenter = simulation.terrain.heightmapIndexToWorldCenter(coords.x, coords.y);
            Rectangle2x<SimScalar> cellRectangle(
                Vector2x<SimScalar>(cellCenter.x, cellCenter.z),
                Vector2x<SimScalar>(MapTerrain::HeightTileWidthInWorldUnits / 2_ss, MapTerrain::HeightTileHeightInWorldUnits / 2_ss));
            auto cellDistanceSquared = cellRectangle.distanceSquared(Vector2x<SimScalar>(position.x, position.z));
            if (cellDistanceSquared > radiusSquared)
            {
                return;
            }

            // check if a unit (or feature) is there
            auto occupiedType = simulation.occupiedGrid.get(coords);
            auto u = match(
                occupiedType.occupiedType,
                [&](const OccupiedUnit& u) { return std::optional(u.id); },
                [&](const auto&) { return std::optional<UnitId>(); });
            if (!u && occupiedType.buildingCell && !occupiedType.buildingCell->passable)
            {
                u = occupiedType.buildingCell->unit;
            }
            if (!u)
            {
                return;
            }

            // check if the unit was seen/mark as seen
            auto pair = seenUnits.insert(*u);
            if (!pair.second) // the unit was already present
            {
                return;
            }

            const auto& unit = simulation.getUnit(*u);

            // skip dead units
            if (unit.isDead())
            {
                return;
            }

            // add in the third dimension component to distance,
            // check if we are still in range
            auto unitDistanceSquared = createBoundingBox(unit).distanceSquared(position);
            if (unitDistanceSquared > radiusSquared)
            {
                return;
            }

            // apply appropriate damage
            auto damageScale = std::clamp(1_ss - (rweSqrt(unitDistanceSquared) / radius), 0_ss, 1_ss);
            auto rawDamage = projectile.getDamage(unit.unitType);
            auto scaledDamage = simScalarToUInt(SimScalar(rawDamage) * damageScale);
            applyDamage(*u, scaledDamage);
        });
    }

    void GameScene::applyDamage(UnitId unitId, unsigned int damagePoints)
    {
        auto& unit = simulation.getUnit(unitId);
        if (unit.hitPoints <= damagePoints)
        {
            killUnit(unitId);
        }
        else
        {
            unit.hitPoints -= damagePoints;
        }
    }

    void GameScene::createLightSmoke(const Vector3f& position)
    {
        spawnSmoke(position, "FX", "smoke 1", ExplosionFinishTimeEndOfFrames(), GameTime(2));
    }

    void GameScene::createWeaponSmoke(const Vector3f& position)
    {
        auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 1");
        spawnSmoke(position, "FX", "smoke 1", ExplosionFinishTimeFixedTime{simulation.gameTime + GameTime(30)}, GameTime(15));
    }

    void GameScene::activateUnit(UnitId unitId)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().activate();
            playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Activate);

            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && *selectedUnit == unitId)
            {
                onOff.next(true);
            }
        }
    }

    void GameScene::deactivateUnit(UnitId unitId)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().deactivate();
            playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Deactivate);

            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && *selectedUnit == unitId)
            {
                onOff.next(false);
            }
        }
    }

    void GameScene::modifyBuildQueue(UnitId unitId, const std::string& unitType, int count)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().modifyBuildQueue(unitType, count);

            updateUnconfirmedBuildQueueDelta(unitId, unitType, -count);
            refreshBuildGuiTotal(unitId, unitType);
        }
    }

    void GameScene::setBuildStance(UnitId unitId, bool value)
    {
        getUnit(unitId).inBuildStance = value;
    }

    void GameScene::setYardOpen(UnitId unitId, bool value)
    {
        simulation.trySetYardOpen(unitId, value);
    }

    void GameScene::setBuggerOff(UnitId unitId, bool value)
    {
        if (value)
        {
            simulation.emitBuggerOff(unitId);
        }
    }

    struct CorpseSpawnInfo
    {
        std::string featureName;
        SimVector position;
        SimAngle rotation;
    };

    void GameScene::deleteDeadUnits()
    {
        std::vector<CorpseSpawnInfo> corpsesToSpawn;

        for (auto it = simulation.units.begin(); it != simulation.units.end();)
        {
            const auto& unit = it->second;
            auto deadState = std::get_if<Unit::LifeStateDead>(&unit.lifeState);
            if (deadState == nullptr)
            {
                ++it;
                continue;
            }

            const auto& unitFbi = unitDatabase.getUnitInfo(unit.unitType);
            if (!unitFbi.corpse.empty())
            {
                corpsesToSpawn.push_back(CorpseSpawnInfo{
                    unitFbi.corpse,
                    unit.position,
                    unit.rotation});
            }

            deselectUnit(it->first);

            if (hoveredUnit && *hoveredUnit == it->first)
            {
                hoveredUnit = std::nullopt;
            }

            auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
            auto footprintRegion = simulation.occupiedGrid.tryToRegion(footprintRect);
            assert(!!footprintRegion);
            if (unit.isMobile)
            {
                simulation.occupiedGrid.forEach(*footprintRegion, [](auto& cell) {
                    cell.occupiedType = OccupiedNone();
                });
            }
            else
            {
                simulation.occupiedGrid.forEach(*footprintRegion, [&](auto& cell) {
                    if (cell.buildingCell && cell.buildingCell->unit == it->first)
                    {
                        cell.buildingCell = std::nullopt;
                    }
                });
            }


            unitGuiInfos.erase(it->first);
            it = simulation.units.erase(it);
        }

        for (const auto& spawnInfo : corpsesToSpawn)
        {
            trySpawnFeature(spawnInfo.featureName, spawnInfo.position, spawnInfo.rotation);
        }
    }

    void GameScene::deleteDeadProjectiles()
    {
        for (auto it = simulation.projectiles.begin(); it != simulation.projectiles.end();)
        {
            const auto& projectile = it->second;
            if (projectile.isDead)
            {
                it = simulation.projectiles.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void GameScene::spawnNewUnits()
    {
        for (const auto& unitId : simulation.unitCreationRequests)
        {
            auto unit = tryGetUnit(unitId);
            if (!unit)
            {
                continue;
            }

            if (auto s = std::get_if<CreatingUnitState>(&unit->get().behaviourState); s != nullptr)
            {

                if (!std::holds_alternative<UnitCreationStatusPending>(s->status))
                {
                    continue;
                }

                auto newUnitId = spawnUnit(s->unitType, s->owner, s->position, std::nullopt);
                if (!newUnitId)
                {
                    s->status = UnitCreationStatusFailed();
                    continue;
                }

                playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Build);

                s->status = UnitCreationStatusDone{*newUnitId};
            }

            if (auto s = std::get_if<FactoryStateCreatingUnit>(&unit->get().factoryState); s != nullptr)
            {
                if (!std::holds_alternative<UnitCreationStatusPending>(s->status))
                {
                    continue;
                }

                auto newUnitId = spawnUnit(s->unitType, s->owner, s->position, s->rotation);
                if (!newUnitId)
                {
                    s->status = UnitCreationStatusFailed();
                    continue;
                }

                s->status = UnitCreationStatusDone{*newUnitId};
            }
        }

        simulation.unitCreationRequests.clear();
    }

    BoundingBox3x<SimScalar> GameScene::createBoundingBox(const Unit& unit) const
    {
        auto footprint = simulation.computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto min = SimVector(SimScalar(footprint.x), unit.position.y, SimScalar(footprint.y));
        auto max = SimVector(SimScalar(footprint.x + footprint.width), unit.position.y + unit.height, SimScalar(footprint.y + footprint.height));
        auto worldMin = simulation.terrain.heightmapToWorldSpace(min);
        auto worldMax = simulation.terrain.heightmapToWorldSpace(max);
        return BoundingBox3x<SimScalar>::fromMinMax(worldMin, worldMax);
    }

    void GameScene::killUnit(UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);

        unit.markAsDead();

        // TODO: spawn debris particles (from Killed script)
        if (unit.explosionWeapon)
        {
            auto impactType = unit.position.y < simulation.terrain.getSeaLevel() ? ImpactType::Water : ImpactType::Normal;
            auto projectile = simulation.createProjectileFromWeapon(unit.owner, *unit.explosionWeapon, unit.position, SimVector(0_ss, -1_ss, 0_ss), 0_ss);
            doProjectileImpact(projectile, impactType);
        }
    }

    void GameScene::quietlyKillUnit(UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);

        unit.markAsDeadNoCorpse();
    }

    void GameScene::killPlayer(PlayerId playerId)
    {
        simulation.getPlayer(playerId).status = GamePlayerStatus::Dead;
        for (auto& p : simulation.units)
        {
            auto& unit = p.second;
            if (unit.isDead())
            {
                continue;
            }

            if (!unit.isOwnedBy(playerId))
            {
                continue;
            }

            killUnit(p.first);
        }
    }

    void GameScene::processActions()
    {
        for (auto& a : actions)
        {
            if (!a)
            {
                continue;
            }

            if (sceneTime < a->triggerTime)
            {
                continue;
            }

            a->callback();
            a = std::nullopt;
        }
    }

    void GameScene::processPlayerCommands(const std::vector<std::pair<PlayerId, std::vector<PlayerCommand>>>& commands)
    {
        for (const auto& [_, playerCommands] : commands)
        {
            for (const auto& command : playerCommands)
            {
                processPlayerCommand(command);
            }
        }
    }

    void GameScene::attachOrdersMenuEventHandlers()
    {
        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "ATTACK"))
        {
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) {
                p.setToggledOn(std::holds_alternative<AttackCursorMode>(v));
            }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "MOVE"))
        {
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) {
                p.setToggledOn(std::holds_alternative<MoveCursorMode>(v));
            }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "DEFEND"))
        {
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) {
                p.setToggledOn(std::holds_alternative<GuardCursorMode>(v));
            }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "FIREORD"))
        {
            p->get().addSubscription(fireOrders.subscribe([&p = p->get()](const auto& v) {
                switch (v)
                {
                    case UnitFireOrders::HoldFire:
                        p.setStage(0);
                        break;
                    case UnitFireOrders::ReturnFire:
                        p.setStage(1);
                        break;
                    case UnitFireOrders::FireAtWill:
                        p.setStage(2);
                        break;
                    default:
                        throw std::logic_error("Invalid FireOrders value");
                }
            }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "ONOFF"))
        {
            p->get().addSubscription(onOff.subscribe([&p = p->get()](const auto& v) {
                p.setStage(v ? 1 : 0);
            }));
        }

        currentPanel->groupMessages().subscribe([this](const auto& msg) {
            if (auto activateMessage = std::get_if<ActivateMessage>(&msg.message); activateMessage != nullptr)
            {
                onMessage(msg.controlName, activateMessage->type);
            }
        });
    }

    UnitFireOrders nextFireOrders(UnitFireOrders orders)
    {
        switch (orders)
        {
            case UnitFireOrders::HoldFire:
                return UnitFireOrders::ReturnFire;
            case UnitFireOrders::ReturnFire:
                return UnitFireOrders::FireAtWill;
            case UnitFireOrders::FireAtWill:
                return UnitFireOrders::HoldFire;
            default:
                throw std::logic_error("Invalid UnitFireOrders value");
        }
    }

    void GameScene::onMessage(const std::string& message, ActivateMessage::Type type)
    {
        if (matchesWithSidePrefix("ATTACK", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (std::holds_alternative<AttackCursorMode>(cursorMode.getValue()))
            {
                cursorMode.next(NormalCursorMode());
            }
            else
            {
                cursorMode.next(AttackCursorMode());
            }
        }
        else if (matchesWithSidePrefix("MOVE", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (std::holds_alternative<MoveCursorMode>(cursorMode.getValue()))
            {
                cursorMode.next(NormalCursorMode());
            }
            else
            {
                cursorMode.next(MoveCursorMode());
            }
        }
        else if (matchesWithSidePrefix("DEFEND", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (std::holds_alternative<GuardCursorMode>(cursorMode.getValue()))
            {
                cursorMode.next(NormalCursorMode());
            }
            else
            {
                cursorMode.next(GuardCursorMode());
            }
        }
        else if (matchesWithSidePrefix("STOP", message))
        {
            if (sounds.immediateOrders)
            {
                sceneContext.audioService->playSound(*sounds.immediateOrders);
            }

            for (const auto& selectedUnit : selectedUnits)
            {
                cursorMode.next(NormalCursorMode());
                localPlayerStopUnit(selectedUnit);
            }
        }
        else if (matchesWithSidePrefix("FIREORD", message))
        {
            if (sounds.setFireOrders)
            {
                sceneContext.audioService->playSound(*sounds.setFireOrders);
            }

            for (const auto& selectedUnit : selectedUnits)
            {
                // FIXME: should set all to a consistent single fire order rather than advancing all
                auto& u = getUnit(selectedUnit);
                auto newFireOrders = nextFireOrders(u.fireOrders);
                localPlayerSetFireOrders(selectedUnit, newFireOrders);
            }
        }
        else if (matchesWithSidePrefix("ONOFF", message))
        {
            if (sounds.immediateOrders)
            {
                sceneContext.audioService->playSound(*sounds.immediateOrders);
            }

            for (const auto& selectedUnit : selectedUnits)
            {
                auto& u = getUnit(selectedUnit);
                auto newOnOff = !u.activated;
                localPlayerSetOnOff(selectedUnit, newOnOff);
            }
        }
        else if (matchesWithSidePrefix("NEXT", message))
        {
            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                if (sounds.nextBuildMenu)
                {
                    sceneContext.audioService->playSound(*sounds.nextBuildMenu);
                }

                const auto& unit = getUnit(*selectedUnit);
                auto& guiInfo = unitGuiInfos.at(*selectedUnit);
                auto pages = unitFactory.getBuildPageCount(unit.unitType);
                guiInfo.currentBuildPage = (guiInfo.currentBuildPage + 1) % pages;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1), *buildPanelDefinition, unit.getBuildQueueTotals()));
                }
            }
        }
        else if (matchesWithSidePrefix("PREV", message))
        {
            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                if (sounds.nextBuildMenu)
                {
                    sceneContext.audioService->playSound(*sounds.nextBuildMenu);
                }

                const auto& unit = getUnit(*selectedUnit);
                auto& guiInfo = unitGuiInfos.at(*selectedUnit);
                auto pages = unitFactory.getBuildPageCount(unit.unitType);
                assert(pages != 0);
                guiInfo.currentBuildPage = guiInfo.currentBuildPage == 0 ? pages - 1 : guiInfo.currentBuildPage - 1;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1), *buildPanelDefinition, unit.getBuildQueueTotals()));
                }
            }
        }
        else if (matchesWithSidePrefix("BUILD", message))
        {
            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                if (sounds.buildButton)
                {
                    sceneContext.audioService->playSound(*sounds.buildButton);
                }

                const auto& unit = getUnit(*selectedUnit);
                auto& guiInfo = unitGuiInfos.at(*selectedUnit);
                guiInfo.section = UnitGuiInfo::Section::Build;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1), *buildPanelDefinition, unit.getBuildQueueTotals()));
                }
            }
        }
        else if (matchesWithSidePrefix("ORDERS", message))
        {
            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                if (sounds.ordersButton)
                {
                    sceneContext.audioService->playSound(*sounds.ordersButton);
                }

                auto& guiInfo = unitGuiInfos.at(*selectedUnit);
                guiInfo.section = UnitGuiInfo::Section::Orders;

                const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
                setNextPanel(uiFactory.panelFromGuiFile(sidePrefix + "GEN"));
            }
        }
        else if (unitFactory.isValidUnitType(message))
        {
            if (sounds.addBuild)
            {
                sceneContext.audioService->playSound(*sounds.addBuild);
            }

            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                const auto& unit = getUnit(*selectedUnit);
                if (unit.isMobile)
                {
                    cursorMode.next(BuildCursorMode{message});
                }
                else
                {
                    int count = (isShiftDown() ? 5 : 1) * (type == ActivateMessage::Type::Primary ? 1 : -1);
                    localPlayerModifyBuildQueue(*selectedUnit, message, count);
                }
            }
        }
    }

    bool GameScene::matchesWithSidePrefix(const std::string& suffix, const std::string& value) const
    {
        for (const auto& side : (*sceneContext.sideData | boost::adaptors::map_values))
        {
            if (side.namePrefix + suffix == value)
            {
                return true;
            }
        }

        return false;
    }

    std::optional<UnitId> GameScene::getSingleSelectedUnit() const
    {
        return selectedUnits.size() == 1
            ? std::make_optional(*selectedUnits.begin())
            : std::nullopt;
    }

    void GameScene::selectUnitsInBandbox(const DiscreteRect& box)
    {
        const auto cameraPos = worldCameraState.getRoundedPosition();
        auto cameraBox = box.translate(-cameraPos.x, -cameraPos.z);
        const auto& matrix = computeViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        std::unordered_set<UnitId> units;

        for (const auto& e : simulation.units)
        {
            if (!e.second.isSelectableBy(localPlayerId))
            {
                continue;
            }

            const auto& worldPos = e.second.position;
            auto clipPos = matrix * simVectorToFloat(worldPos);
            Point viewportPos = worldViewport.toViewportSpace(clipPos.x, clipPos.y);
            if (!cameraBox.contains(viewportPos))
            {
                continue;
            }

            units.insert(e.first);
        }

        if (isShiftDown())
        {
            toggleUnitSelection(units);
        }
        else
        {
            replaceUnitSelection(units);
        }
    }

    void GameScene::toggleUnitSelection(const rwe::UnitId& unitId)
    {
        auto it = selectedUnits.find(unitId);
        if (it != selectedUnits.end())
        {
            deselectUnit(unitId);
            return;
        }

        selectAdditionalUnit(unitId);
    }

    void GameScene::toggleUnitSelection(const std::unordered_set<UnitId>& units)
    {
        std::unordered_set<UnitId> newSelection(selectedUnits);
        for (const auto& unitId : units)
        {
            auto [it, inserted] = newSelection.insert(unitId);
            if (!inserted)
            {
                newSelection.erase(it);
            }
        }

        replaceUnitSelection(newSelection);
    }

    void GameScene::selectAdditionalUnit(const rwe::UnitId& unitId)
    {
        selectedUnits.insert(unitId);

        const auto& unit = getUnit(unitId);
        auto selectionSound = getSound(unitDatabase, meshDatabase, unit.unitType, UnitSoundType::Select1);
        if (selectionSound)
        {
            playUiSound(*selectionSound);
        }

        onSelectedUnitsChanged();
    }

    void GameScene::replaceUnitSelection(const UnitId& unitId)
    {
        selectedUnits.clear();
        selectAdditionalUnit(unitId);
    }

    void GameScene::deselectUnit(const UnitId& unitId)
    {
        selectedUnits.erase(unitId);
        onSelectedUnitsChanged();
    }

    void GameScene::clearUnitSelection()
    {
        selectedUnits.clear();
        onSelectedUnitsChanged();
    }

    void GameScene::replaceUnitSelection(const std::unordered_set<UnitId>& units)
    {
        selectedUnits = units;

        if (selectedUnits.size() == 1)
        {
            const auto& unit = getUnit(*units.begin());
            auto selectionSound = getSound(unitDatabase, meshDatabase, unit.unitType, UnitSoundType::Select1);
            if (selectionSound)
            {
                playUiSound(*selectionSound);
            }
        }
        else if (selectedUnits.size() > 0)
        {
            if (sounds.selectMultipleUnits)
            {
                playUiSound(*sounds.selectMultipleUnits);
            }
        }

        onSelectedUnitsChanged();
    }

    void GameScene::onSelectedUnitsChanged()
    {
        if (selectedUnits.empty())
        {
            const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
            setNextPanel(uiFactory.panelFromGuiFile(sidePrefix + "MAIN2"));
        }
        else if (auto unitId = getSingleSelectedUnit(); unitId)
        {
            const auto& unit = getUnit(*unitId);
            fireOrders.next(unit.fireOrders);
            onOff.next(unit.activated);

            const auto& guiInfo = getGuiInfo(*unitId);
            auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
            if (guiInfo.section == UnitGuiInfo::Section::Build && buildPanelDefinition)
            {
                setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1), *buildPanelDefinition, unit.getBuildQueueTotals()));
            }
            else
            {
                const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
                setNextPanel(uiFactory.panelFromGuiFile(sidePrefix + "GEN"));
            }
        }
        else
        {
            const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
            setNextPanel(uiFactory.panelFromGuiFile(sidePrefix + "GEN"));
        }
    }

    const UnitGuiInfo& GameScene::getGuiInfo(const UnitId& unitId) const
    {
        auto it = unitGuiInfos.find(unitId);
        if (it == unitGuiInfos.end())
        {
            throw std::logic_error("Gui info not found for unit " + std::to_string(unitId.value));
        }
        return it->second;
    }

    void GameScene::setNextPanel(std::unique_ptr<UiPanel>&& panel)
    {
        nextPanel = std::move(panel);
    }

    void GameScene::refreshBuildGuiTotal(UnitId unitId, const std::string& unitType)
    {
        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit == unitId)
        {
            const auto& unit = getUnit(*selectedUnit);
            auto total = unit.getBuildQueueTotal(unitType) + getUnconfirmedBuildQueueCount(unitId, unitType);
            auto button = currentPanel->find<UiStagedButton>(unitType);
            if (button)
            {
                button->get().setLabel(total > 0 ? "+" + std::to_string(total) : "");
            }
        }
    }

    void GameScene::updateUnconfirmedBuildQueueDelta(UnitId unitId, const std::string& unitType, int count)
    {
        auto it = unconfirmedBuildQueueDelta.find(unitId);
        if (it == unconfirmedBuildQueueDelta.end())
        {
            unconfirmedBuildQueueDelta.emplace(unitId, std::unordered_map<std::string, int>{{unitType, count}});
        }
        else
        {
            auto it2 = it->second.find(unitType);
            if (it2 == it->second.end())
            {
                it->second.emplace(unitType, count);
            }
            else
            {
                int newTotal = it2->second + count;
                if (newTotal != 0)
                {
                    it2->second = newTotal;
                }
                else
                {
                    it->second.erase(it2);
                }
            }
        }
    }

    int GameScene::getUnconfirmedBuildQueueCount(UnitId unitId, const std::string& unitType) const
    {
        auto it = unconfirmedBuildQueueDelta.find(unitId);
        if (it == unconfirmedBuildQueueDelta.end())
        {
            return 0;
        }

        auto it2 = it->second.find(unitType);
        if (it2 == it->second.end())
        {
            return 0;
        }

        return it2->second;
    }

    std::unique_ptr<UiPanel> GameScene::createBuildPanel(const std::string& guiName, const std::vector<GuiEntry>& buildPanelDefinition, const std::unordered_map<std::string, int>& totals)
    {
        auto panel = uiFactory.panelFromGuiFile(guiName, buildPanelDefinition);
        for (const auto& e : totals)
        {
            auto button = panel->find<UiStagedButton>(e.first);
            if (button)
            {
                button->get().setLabel("+" + std::to_string(e.second));
            }
        }

        return panel;
    }

    void GameScene::processPlayerCommand(const PlayerCommand& playerCommand)
    {
        match(
            playerCommand,
            [&](const PlayerUnitCommand& c) {
                processUnitCommand(c);
            },
            [](const PlayerPauseGameCommand&) {
                // TODO
            },
            [](const PlayerUnpauseGameCommand&) {
                // TODO
            });
    }

    void GameScene::processUnitCommand(const PlayerUnitCommand& unitCommand)
    {
        match(
            unitCommand.command,
            [&](const PlayerUnitCommand::IssueOrder& c) {
                switch (c.issueKind)
                {
                    case PlayerUnitCommand::IssueOrder::IssueKind::Immediate:
                        issueUnitOrder(unitCommand.unit, c.order);
                        break;
                    case PlayerUnitCommand::IssueOrder::IssueKind::Queued:
                        enqueueUnitOrder(unitCommand.unit, c.order);
                        break;
                }
            },
            [&](const PlayerUnitCommand::ModifyBuildQueue& c) {
                modifyBuildQueue(unitCommand.unit, c.unitType, c.count);
            },
            [&](const PlayerUnitCommand::Stop&) {
                stopUnit(unitCommand.unit);
            },
            [&](const PlayerUnitCommand::SetFireOrders& c) {
                setFireOrders(unitCommand.unit, c.orders);
            },
            [&](const PlayerUnitCommand::SetOnOff& c) {
                if (c.on)
                {
                    activateUnit(unitCommand.unit);
                }
                else
                {
                    deactivateUnit(unitCommand.unit);
                }
            });
    }

    bool GameScene::leftClickMode() const
    {
        return sceneContext.globalConfig->leftClickInterfaceMode;
    }

    void GameScene::spawnExplosion(const Vector3f& position, const AnimLocation& anim)
    {
        Explosion exp;
        exp.position = position;
        exp.explosionGaf = anim.gafName;
        exp.explosionAnim = anim.animName;
        exp.startTime = simulation.gameTime;
        exp.finishTime = ExplosionFinishTimeEndOfFrames();
        exp.frameDuration = GameTime(2);

        explosions.push_back(exp);
    }

    void GameScene::spawnFlash(const Vector3f& position)
    {
        FlashEffect flash;
        flash.position = position;
        flash.startTime = simulation.gameTime;
        flash.duration = GameTime(15);
        flash.maxRadius = 30.0f;
        flash.color = Vector3f(1.0f, 1.0f, 1.0f);
        flash.maxIntensity = 1.0f;
        flashes.push_back(flash);
    }

    void GameScene::spawnSmoke(const Vector3f& position, const std::string& gaf, const std::string& anim, ExplosionFinishTime duration, GameTime frameDuration)
    {
        Explosion exp;
        exp.position = position;
        exp.explosionGaf = gaf;
        exp.explosionAnim = anim;
        exp.startTime = simulation.gameTime;
        exp.finishTime = duration;
        exp.frameDuration = frameDuration;
        exp.translucent = true;
        exp.floats = true;

        explosions.push_back(exp);
    }

    void GameScene::recreateWorldRenderTextures()
    {
        worldFrameBuffer = sceneContext.graphics->createFrameBuffer(worldViewport.width(), worldViewport.height());
        dodgeMask = sceneContext.graphics->createEmptyTexture(worldViewport.width(), worldViewport.height());
    }
}
