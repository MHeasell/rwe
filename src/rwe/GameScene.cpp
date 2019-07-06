#include "GameScene.h"
#include <boost/range/adaptor/map.hpp>
#include <functional>
#include <rwe/Mesh.h>
#include <rwe/overloaded.h>
#include <rwe/resource_io.h>
#include <rwe/ui/UiStagedButton.h>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace rwe
{
    bool laserCollides(const GameSimulation& sim, const LaserProjectile& laser, const OccupiedCell& cellValue)
    {
        auto collidesWithOccupiedCell = match(
            cellValue.occupiedType,
            [&](const OccupiedUnit& v) {
                const auto& unit = sim.getUnit(v.id);

                if (unit.isOwnedBy(laser.owner))
                {
                    return false;
                }

                // ignore if the laser is above or below the unit
                if (laser.position.y < unit.position.y || laser.position.y > unit.position.y + unit.height)
                {
                    return false;
                }

                return true;
            },
            [&](const OccupiedFeature& v) {
                const auto& feature = sim.getFeature(v.id);

                // ignore if the laser is above or below the feature
                if (laser.position.y < feature.position.y || laser.position.y > feature.position.y + feature.height)
                {
                    return false;
                }

                return true;
            },

            [&](const OccupiedNone&) { return false; });

        if (collidesWithOccupiedCell)
        {
            return true;
        }

        if (cellValue.buildingCell && !cellValue.buildingCell->passable)
        {
            const auto& unit = sim.getUnit(cellValue.buildingCell->unit);

            if (unit.isOwnedBy(laser.owner))
            {
                return false;
            }

            // ignore if the laser is above or below the unit
            if (laser.position.y < unit.position.y || laser.position.y > unit.position.y + unit.height)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    const Rectangle2f GameScene::minimapViewport = Rectangle2f::fromTopLeft(0.0f, 0.0f, GuiSizeLeft, GuiSizeLeft);

    GameScene::GameScene(const SceneContext&                     sceneContext,
                         std::unique_ptr<PlayerCommandService>&& playerCommandService,
                         RenderService&&                         worldRenderService,
                         UiRenderService&&                       worldUiRenderService,
                         UiRenderService&&                       chromeUiRenderService,
                         GameSimulation&&                        simulation,
                         MovementClassCollisionService&&         collisionService,
                         UnitDatabase&&                          unitDatabase,
                         MeshService&&                           meshService,
                         std::unique_ptr<GameNetworkService>&&   gameNetworkService,
                         const std::shared_ptr<Sprite>&          minimap,
                         const std::shared_ptr<SpriteSeries>&    minimapDots,
                         const std::shared_ptr<Sprite>&          minimapDotHighlight,
                         InGameSoundsInfo                        sounds,
                         const std::shared_ptr<SpriteSeries>&    guiFont,
                         PlayerId                                localPlayerId,
                         TdfBlock*                               audioLookup)
        : sceneContext(sceneContext),
          worldViewport(ViewportService(GuiSizeLeft,
                                        GuiSizeTop,
                                        sceneContext.viewportService->width() - GuiSizeLeft - GuiSizeRight,
                                        sceneContext.viewportService->height() - GuiSizeTop - GuiSizeBottom)),
          playerCommandService(std::move(playerCommandService)),
          worldRenderService(std::move(worldRenderService)),
          worldUiRenderService(std::move(worldUiRenderService)),
          chromeUiRenderService(std::move(chromeUiRenderService)),
          simulation(std::move(simulation)),
          collisionService(std::move(collisionService)),
          unitFactory(sceneContext.textureService,
                      std::move(unitDatabase),
                      std::move(meshService),
                      &this->collisionService,
                      sceneContext.palette,
                      sceneContext.guiPalette),
          gameNetworkService(std::move(gameNetworkService)),
          pathFindingService(&this->simulation, &this->collisionService),
          unitBehaviorService(this, &pathFindingService, &this->collisionService, &this->unitFactory),
          cobExecutionService(),
          minimap(minimap),
          minimapDots(minimapDots),
          minimapDotHighlight(minimapDotHighlight),
          minimapRect(minimapViewport.scaleToFit(this->minimap->bounds)),
          sounds(std::move(sounds)),
          guiFont(guiFont),
          localPlayerId(localPlayerId),
          audioLookup(audioLookup),
          uiFactory(sceneContext.textureService,
                    sceneContext.audioService,
                    audioLookup,
                    sceneContext.vfs,
                    sceneContext.viewportService->width(),
                    sceneContext.viewportService->height())
    {
    }

    void GameScene::init()
    {
        const auto& sidePrefix = sceneContext.sideData->at(getPlayer(localPlayerId).side).namePrefix;
        currentPanel           = uiFactory.panelFromGuiFile(sidePrefix + "MAIN2");

        sceneContext.audioService->reserveChannels(reservedChannelsCount);
        gameNetworkService->start();
    }

    void GameScene::render()
    {
        const auto& localSideData = sceneContext.sideData->at(getPlayer(localPlayerId).side);

        sceneContext.graphics->setViewport(
            0, 0, sceneContext.viewportService->width(), sceneContext.viewportService->height());
        sceneContext.graphics->disableDepthBuffer();

        renderMinimap();

        // render top bar
        const auto& intGafName = localSideData.intGaf;
        auto        topPanelBackground
            = sceneContext.textureService->tryGetGafEntry("anims/" + intGafName + ".GAF", "PANELTOP");
        auto bottomPanelBackground
            = sceneContext.textureService->tryGetGafEntry("anims/" + intGafName + ".GAF", "PANELBOT");
        float topXBuffer = GuiSizeLeft;
        if (topPanelBackground)
        {
            const auto& sprite = *(*topPanelBackground)->sprites.at(0);
            chromeUiRenderService.drawSpriteAbs(topXBuffer, 0, sprite);
            topXBuffer += sprite.bounds.width();
        }
        if (bottomPanelBackground)
        {
            while (topXBuffer < sceneContext.viewportService->width())
            {
                const auto& sprite = *(*bottomPanelBackground)->sprites.at(0);
                chromeUiRenderService.drawSpriteAbs(topXBuffer, 0.0f, sprite);
                topXBuffer += sprite.bounds.width();
            }
        }

        auto logos = sceneContext.textureService->tryGetGafEntry("textures/LOGOS.GAF", "32xlogos");
        if (logos)
        {
            auto        playerColorIndex = getPlayer(localPlayerId).color;
            const auto& rect             = localSideData.logo.toDiscreteRect();
            chromeUiRenderService.drawSpriteAbs(
                rect.x, rect.y, rect.width, rect.height, *(*logos)->sprites.at(playerColorIndex.value));
        }

        // draw energy bar
        {
            const auto& rect        = localSideData.energyBar.toDiscreteRect();
            const auto& localPlayer = getPlayer(localPlayerId);
            auto rectWidth = (rect.width * std::max(Energy(0), localPlayer.energy).value) / localPlayer.maxEnergy.value;
            const auto& colorIndex = localSideData.energyColor;
            const auto& color      = sceneContext.palette->at(colorIndex);
            chromeUiRenderService.fillColor(rect.x, rect.y, rectWidth, rect.height, color);
        }
        {
            const auto& rect = localSideData.energy0;
            chromeUiRenderService.drawText(rect.x1, rect.y1, formatResource(Energy(0)), *guiFont);
        }
        {
            const auto& rect = localSideData.energyMax;
            auto        text = formatResource(getPlayer(localPlayerId).maxEnergy);
            chromeUiRenderService.drawTextAlignRight(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.energyNum;
            auto        text = formatResource(std::max(Energy(0), getPlayer(localPlayerId).energy));
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.energyProduced;
            auto        text = formatResourceDelta(getPlayer(localPlayerId).energyProductionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(83, 223, 79));
        }
        {
            const auto& rect = localSideData.energyConsumed;
            auto        text = formatResourceDelta(getPlayer(localPlayerId).previousDesiredEnergyConsumptionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(255, 71, 0));
        }

        // draw metal bar
        {
            const auto& rect        = localSideData.metalBar.toDiscreteRect();
            const auto& localPlayer = getPlayer(localPlayerId);
            auto rectWidth = (rect.width * std::max(Metal(0), localPlayer.metal).value) / localPlayer.maxMetal.value;
            const auto& colorIndex = localSideData.metalColor;
            const auto& color      = sceneContext.palette->at(colorIndex);
            chromeUiRenderService.fillColor(rect.x, rect.y, rectWidth, rect.height, color);
        }
        {
            const auto& rect = localSideData.metal0;
            chromeUiRenderService.drawText(rect.x1, rect.y1, "0", *guiFont);
        }
        {
            const auto& rect = localSideData.metalMax;
            auto        text = formatResource(getPlayer(localPlayerId).maxMetal);
            chromeUiRenderService.drawTextAlignRight(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.metalNum;
            auto        text = formatResource(std::max(Metal(0), getPlayer(localPlayerId).metal));
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont);
        }
        {
            const auto& rect = localSideData.metalProduced;
            auto        text = formatResourceDelta(getPlayer(localPlayerId).metalProductionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(83, 223, 79));
        }
        {
            const auto& rect = localSideData.metalConsumed;
            auto        text = formatResourceDelta(getPlayer(localPlayerId).previousDesiredMetalConsumptionBuffer);
            chromeUiRenderService.drawText(rect.x1, rect.y1, text, *guiFont, Color(255, 71, 0));
        }

        // render bottom bar
        float bottomXBuffer = GuiSizeLeft;
        if (bottomPanelBackground)
        {
            while (bottomXBuffer < sceneContext.viewportService->width())
            {
                const auto& sprite = *(*bottomPanelBackground)->sprites.at(0);
                chromeUiRenderService.drawSpriteAbs(bottomXBuffer, worldViewport.bottom(), sprite);
                bottomXBuffer += sprite.bounds.width();
            }
        }

        auto extraBottom = sceneContext.viewportService->height() - 480;
        if (auto hoveredUnit = std::get_if<UnitId>(&elementOfInterest))
        {
            const auto& unit = getUnit(*hoveredUnit);
            if (logos)
            {
                const auto& rect  = localSideData.logo2.toDiscreteRect();
                const auto& color = *(*logos)->sprites.at(getPlayer(unit.owner).color.value);
                chromeUiRenderService.drawSpriteAbs(rect.x, extraBottom + rect.y, rect.width, rect.height, color);
            }

            {
                const auto& rect       = localSideData.unitName;
                const auto& playerName = getPlayer(unit.owner).name;
                const auto& text       = unit.showPlayerName && playerName ? *playerName : unit.name;
                chromeUiRenderService.drawTextCenteredX(rect.x1, extraBottom + rect.y1, text, *guiFont);
            }

            if (unit.isOwnedBy(localPlayerId) || !unit.hideDamage)
            {
                const auto& rect = localSideData.damageBar.toDiscreteRect();
                chromeUiRenderService.drawHealthBar2(rect.x,
                                                     extraBottom + rect.y,
                                                     rect.width,
                                                     rect.height,
                                                     static_cast<float>(unit.hitPoints)
                                                         / static_cast<float>(unit.maxHitPoints));
            }

            if (unit.isOwnedBy(localPlayerId))
            {
                {
                    const auto& rect = localSideData.unitMetalMake;
                    auto        text = "+" + formatResourceDelta(unit.getMetalMake());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(83, 223, 79));
                }
                {
                    const auto& rect = localSideData.unitMetalUse;
                    auto        text = "-" + formatResourceDelta(unit.getMetalUse());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(255, 71, 0));
                }
                {
                    const auto& rect = localSideData.unitEnergyMake;
                    auto        text = "+" + formatResourceDelta(unit.getEnergyMake());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(83, 223, 79));
                }
                {
                    const auto& rect = localSideData.unitEnergyUse;
                    auto        text = "-" + formatResourceDelta(unit.getEnergyUse());
                    chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont, Color(255, 71, 0));
                }
                {
                    const auto& rect = localSideData.missionText;
                    auto        text = "Standby";
                    chromeUiRenderService.drawTextCenteredX(rect.x1, extraBottom + rect.y1, text, *guiFont);
                }
            }
        }

        currentPanel->render(chromeUiRenderService);
        sceneContext.graphics->enableDepthBuffer();

        auto viewportPos = worldViewport.toOtherViewport(*sceneContext.viewportService, 0, worldViewport.height());
        sceneContext.graphics->setViewport(viewportPos.x,
                                           sceneContext.viewportService->height() - viewportPos.y,
                                           worldViewport.width(),
                                           worldViewport.height());
        renderWorld();

        sceneContext.graphics->setViewport(
            0, 0, sceneContext.viewportService->width(), sceneContext.viewportService->height());
        sceneContext.graphics->disableDepthBuffer();
        sceneContext.cursor->render(chromeUiRenderService);
        sceneContext.graphics->enableDepthBuffer();
    }

    void GameScene::renderMinimap()
    {
        // draw minimap
        chromeUiRenderService.drawSpriteAbs(minimapRect, *minimap);

        auto cameraInverse  = worldRenderService.getCamera().getInverseViewProjectionMatrix();
        auto worldToMinimap = worldToMinimapMatrix(simulation.terrain, minimapRect);

        // draw minimap dots
        for (const auto& [_, unit] : simulation.units)
        {
            auto minimapPos = worldToMinimap * unit.position;
            minimapPos.x    = std::floor(minimapPos.x);
            minimapPos.y    = std::floor(minimapPos.y);
            auto ownerId    = unit.owner;
            auto colorIndex = getPlayer(ownerId).color;
            chromeUiRenderService.drawSprite(minimapPos.x, minimapPos.y, *minimapDots->sprites[colorIndex.value]);
        }
        // highlight the minimap dot for the hovered unit
        if (auto hoveredUnit = std::get_if<UnitId>(&elementOfInterest))
        {
            const auto& unit       = getUnit(*hoveredUnit);
            auto        minimapPos = worldToMinimap * unit.position;
            minimapPos.x           = std::floor(minimapPos.x);
            minimapPos.y           = std::floor(minimapPos.y);
            chromeUiRenderService.drawSprite(minimapPos.x, minimapPos.y, *minimapDotHighlight);
        }

        // draw minimap viewport rectangle
        {
            auto transform  = worldToMinimap * cameraInverse;
            auto bottomLeft = transform * Vector3f(-1.0f, -1.0f, 0.0f);
            auto topRight   = transform * Vector3f(1.0f, 1.0f, 0.0f);

            chromeUiRenderService.drawBoxOutline(std::round(bottomLeft.x),
                                                 std::round(topRight.y),
                                                 std::round(topRight.x - bottomLeft.x),
                                                 std::round(bottomLeft.y - topRight.y),
                                                 Color(247, 227, 103));
        }
    }

    void GameScene::renderWorld()
    {
        sceneContext.graphics->disableDepthBuffer();

        worldRenderService.drawMapTerrain(simulation.terrain);

        worldRenderService.drawFlatFeatureShadows(simulation.features | boost::adaptors::map_values);
        worldRenderService.drawFlatFeatures(simulation.features | boost::adaptors::map_values);

        if (occupiedGridVisible)
        {
            worldRenderService.drawOccupiedGrid(simulation.terrain, simulation.occupiedGrid);
        }

        if (pathfindingVisualisationVisible)
        {
            worldRenderService.drawPathfindingVisualisation(simulation.terrain, pathFindingService.lastPathDebugInfo);
        }

        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && movementClassGridVisible)
        {
            const auto& unit = simulation.getUnit(*selectedUnit);
            if (unit.movementClass)
            {
                const auto& grid = collisionService.getGrid(*unit.movementClass);
                worldRenderService.drawMovementClassCollisionGrid(simulation.terrain, grid);
            }
        }

        for (const auto& selectedUnitId : selectedUnits)
        {
            worldRenderService.drawSelectionRect(getUnit(selectedUnitId));
        }

        worldRenderService.drawUnitShadows(simulation.terrain, simulation.units | boost::adaptors::map_values);

        sceneContext.graphics->enableDepthBuffer();

        auto seaLevel = simulation.terrain.getSeaLevel();
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            worldRenderService.drawUnit(unit, seaLevel, simulation.gameTime.value);
        }

        worldRenderService.drawLasers(simulation.lasers);

        sceneContext.graphics->disableDepthWrites();

        sceneContext.graphics->disableDepthTest();
        worldRenderService.drawStandingFeatureShadows(simulation.features | boost::adaptors::map_values);
        sceneContext.graphics->enableDepthTest();

        worldRenderService.drawStandingFeatures(simulation.features | boost::adaptors::map_values);

        sceneContext.graphics->disableDepthTest();
        worldRenderService.drawExplosions(simulation.gameTime, simulation.explosions);
        sceneContext.graphics->enableDepthTest();

        sceneContext.graphics->enableDepthWrites();

        // in-world UI/overlay rendering
        sceneContext.graphics->disableDepthBuffer();

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

                auto uiPos = worldUiRenderService.getCamera().getInverseViewProjectionMatrix()
                             * worldRenderService.getCamera().getViewProjectionMatrix() * unit.position;
                worldUiRenderService.drawHealthBar(
                    uiPos.x, uiPos.y, static_cast<float>(unit.hitPoints) / static_cast<float>(unit.maxHitPoints));
            }
        }

        // Draw build box outline when a unit is selected to be built
        if (hoverBuildInfo)
        {
            Color color = hoverBuildInfo->isValid ? Color(0, 255, 0) : Color(255, 0, 0);

            auto topLeftWorld
                = simulation.terrain.heightmapIndexToWorldCorner(hoverBuildInfo->rect.x, hoverBuildInfo->rect.y);
            topLeftWorld.y = simulation.terrain.getHeightAt(
                topLeftWorld.x + ((hoverBuildInfo->rect.width * MapTerrain::HeightTileWidthInWorldUnits) / 2.0f),
                topLeftWorld.z + ((hoverBuildInfo->rect.height * MapTerrain::HeightTileHeightInWorldUnits) / 2.0f));

            auto topLeftUi = worldUiRenderService.getCamera().getInverseViewProjectionMatrix()
                             * worldRenderService.getCamera().getViewProjectionMatrix() * topLeftWorld;
            worldUiRenderService.drawBoxOutline(topLeftUi.x,
                                                topLeftUi.y,
                                                hoverBuildInfo->rect.width * MapTerrain::HeightTileWidthInWorldUnits,
                                                hoverBuildInfo->rect.height * MapTerrain::HeightTileHeightInWorldUnits,
                                                color);
        }

        // Draw bandbox selection rectangle
        if (auto normalCursorMode = std::get_if<NormalCursorMode>(&cursorMode.getValue()))
        {
            if (auto selectingState = std::get_if<NormalCursorMode::SelectingState>(&normalCursorMode->state))
            {
                const auto& start          = selectingState->startPosition;
                const auto& camera         = worldRenderService.getCamera();
                const auto  cameraPosition = camera.getPosition();
                Point       cameraRelativeStart(start.x - cameraPosition.x, start.y - cameraPosition.z);

                auto worldViewportPos
                    = sceneContext.viewportService->toOtherViewport(worldViewport, getMousePosition());
                auto rect = DiscreteRect::fromPoints(cameraRelativeStart, worldViewportPos);

                worldUiRenderService.drawBoxOutline(rect.x, rect.y, rect.width, rect.height, Color(255, 255, 255));
                if (rect.width > 2 && rect.height > 2)
                {
                    worldUiRenderService.drawBoxOutline(
                        rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2, Color(0, 0, 0));
                }
            }
        }

        if (cursorTerrainDotVisible)
        {
            // draw a dot where we think the cursor intersects terrain
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            auto intersect = simulation.intersectLineWithTerrain(ray.toLine());
            if (intersect)
            {
                auto cursorTerrainPos = worldUiRenderService.getCamera().getInverseViewProjectionMatrix()
                                        * worldRenderService.getCamera().getViewProjectionMatrix() * (*intersect);
                worldUiRenderService.fillColor(cursorTerrainPos.x - 2, cursorTerrainPos.y - 2, 4, 4, Color(0, 0, 255));

                intersect->y = simulation.terrain.getHeightAt(intersect->x, intersect->z);

                auto heightTestedTerrainPos = worldUiRenderService.getCamera().getInverseViewProjectionMatrix()
                                              * worldRenderService.getCamera().getViewProjectionMatrix() * (*intersect);
                worldUiRenderService.fillColor(
                    heightTestedTerrainPos.x - 2, heightTestedTerrainPos.y - 2, 4, 4, Color(255, 0, 0));
            }
        }

        sceneContext.graphics->enableDepthBuffer();
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
        else if (keysym.sym == SDLK_LSHIFT)
        {
            leftShiftDown = true;
        }
        else if (keysym.sym == SDLK_RSHIFT)
        {
            rightShiftDown = true;
        }
        else if (keysym.sym == SDLK_F8)
        {
            cursorTerrainDotVisible = !cursorTerrainDotVisible;
        }
        else if (keysym.sym == SDLK_F9)
        {
            occupiedGridVisible = !occupiedGridVisible;
        }
        else if (keysym.sym == SDLK_F10)
        {
            pathfindingVisualisationVisible = !pathfindingVisualisationVisible;
        }
        else if (keysym.sym == SDLK_F11)
        {
            movementClassGridVisible = !movementClassGridVisible;
        }
        else if (keysym.scancode == SDL_SCANCODE_GRAVE)
        {
            healthBarsVisible = !healthBarsVisible;
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
                        match(
                            elementOfInterest,
                            [this, &selectedUnit](UnitId hoveredUnitId) {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(hoveredUnitId));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, AttackOrder(hoveredUnitId));
                                    cursorMode.next(NormalCursorMode());
                                }
                            },
                            [this, &selectedUnit](std::optional<Vector3f>& coord) {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(*coord));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, AttackOrder(*coord));
                                    cursorMode.next(NormalCursorMode());
                                }
                            },
                            [](auto) {});
                    }
                },
                [&](const MoveCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        match(
                            elementOfInterest,
                            [this, &selectedUnit](std::optional<Vector3f>& coord) {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(*coord));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, MoveOrder(*coord));
                                    cursorMode.next(NormalCursorMode());
                                }
                            },
                            [](auto) {});
                    }
                },
                [&](const BuildCursorMode& buildCursor) {
                    if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
                    {
                        if (hoverBuildInfo)
                        {
                            if (hoverBuildInfo->isValid)
                            {
                                auto topLeftWorld = simulation.terrain.heightmapIndexToWorldCorner(
                                    hoverBuildInfo->rect.x, hoverBuildInfo->rect.y);
                                auto x
                                    = topLeftWorld.x
                                      + ((hoverBuildInfo->rect.width * MapTerrain::HeightTileWidthInWorldUnits) / 2.0f);
                                auto z = topLeftWorld.z
                                         + ((hoverBuildInfo->rect.height * MapTerrain::HeightTileHeightInWorldUnits)
                                            / 2.0f);
                                auto     y = simulation.terrain.getHeightAt(x, z);
                                Vector3f buildPos(x, y, z);
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(*selectedUnit,
                                                                BuildOrder(buildCursor.unitType, buildPos));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(*selectedUnit,
                                                              BuildOrder(buildCursor.unitType, buildPos));
                                    cursorMode.next(NormalCursorMode());
                                }
                            }
                            else
                            {
                                if (sounds.notOkToBuild)
                                {
                                    playSoundOnSelectChannel(*sounds.notOkToBuild);
                                }
                            }
                        }
                    }
                },
                [&](const NormalCursorMode&) {
                    if (isCursorOverMinimap())
                    {
                        cursorMode.next(NormalCursorMode{NormalCursorMode::DraggingMinimapState()});
                    }
                    else if (isCursorOverWorld())
                    {
                        Point      p(event.x, event.y);
                        auto       worldViewportPos = sceneContext.viewportService->toOtherViewport(worldViewport, p);
                        const auto cameraPosition   = worldRenderService.getCamera().getPosition();
                        Point      originRelativePos(cameraPosition.x + worldViewportPos.x,
                                                cameraPosition.z + worldViewportPos.y);
                        cursorMode.next(NormalCursorMode{NormalCursorMode::SelectingState(originRelativePos)});
                    }
                },
                [&](const RepairCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        match(
                            elementOfInterest,
                            [this, &selectedUnit](UnitId hoveredUnitId) {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, RepairOrder(hoveredUnitId));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, RepairOrder(hoveredUnitId));
                                    cursorMode.next(NormalCursorMode());
                                }
                            },
                            [](auto) {});
                    }
                });
        }
        else if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            match(
                cursorMode.getValue(),
                [&](const AttackCursorMode&) { cursorMode.next(NormalCursorMode()); },
                [&](const MoveCursorMode&) { cursorMode.next(NormalCursorMode()); },
                [&](const BuildCursorMode&) { cursorMode.next(NormalCursorMode()); },
                [&](const RepairCursorMode&) { cursorMode.next(NormalCursorMode()); },
                [&](const NormalCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {

                        match(
                            elementOfInterest,
                            [this, &selectedUnit](const UnitId& id) {
                                if (getUnit(selectedUnit).builder && getUnit(id).isDamaged())
                                {
                                    if (isShiftDown())
                                    {
                                        localPlayerEnqueueUnitOrder(selectedUnit, RepairOrder(id));
                                    }
                                    else
                                    {
                                        localPlayerIssueUnitOrder(selectedUnit, RepairOrder(id));
                                    }
                                }
                                if (isEnemy(id))
                                {
                                    if (isShiftDown())
                                    {
                                        localPlayerEnqueueUnitOrder(selectedUnit, AttackOrder(id));
                                    }
                                    else
                                    {
                                        localPlayerIssueUnitOrder(selectedUnit, AttackOrder(id));
                                    }
                                }
                            },
                            [this, &selectedUnit](const Vector3f& coord) {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, MoveOrder(coord));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, MoveOrder(coord));
                                }
                            },
                            [](auto) {});
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
                            auto  worldViewportPos    = sceneContext.viewportService->toOtherViewport(worldViewport, p);
                            const auto cameraPosition = worldRenderService.getCamera().getPosition();
                            Point      originRelativePos(cameraPosition.x + worldViewportPos.x,
                                                    cameraPosition.z + worldViewportPos.y);

                            if (state.startPosition == originRelativePos)
                            {
                                match(
                                    elementOfInterest,
                                    [this](const UnitId& id) {
                                        if (getUnit(id).isSelectableBy(localPlayerId))
                                        {
                                            replaceUnitSelection(id);
                                        }
                                    },
                                    [this](auto) { clearUnitSelection(); });
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
                        [&](const NormalCursorMode::UpState&) {});
                },
                [&](const auto&) {
                    // do nothing
                });
        }
    }

    void GameScene::onMouseMove(MouseMoveEvent event) { currentPanel->mouseMove(event); }

    void GameScene::onMouseWheel(MouseWheelEvent event) { currentPanel->mouseWheel(event); }

    Rectangle2f computeCameraConstraint(const MapTerrain& terrain, const CabinetCamera& camera)
    {
        auto cameraHalfWidth  = camera.getWidth() / 2.0f;
        auto cameraHalfHeight = camera.getHeight() / 2.0f;

        auto top    = terrain.topInWorldUnits() + cameraHalfHeight;
        auto left   = terrain.leftInWorldUnits() + cameraHalfWidth;
        auto bottom = terrain.bottomCutoffInWorldUnits() - cameraHalfHeight;
        auto right  = terrain.rightCutoffInWorldUnits() - cameraHalfWidth;

        if (left > right)
        {
            auto middle = (left + right) / 2.0f;
            left        = middle;
            right       = middle;
        }

        if (top > bottom)
        {
            auto middle = (top + bottom) / 2.0f;
            top         = middle;
            bottom      = middle;
        }

        return Rectangle2f::fromTLBR(top, left, bottom, right);
    }

    void GameScene::update()
    {
        auto& camera           = worldRenderService.getCamera();
        auto  cameraConstraint = computeCameraConstraint(simulation.terrain, camera);

        // update camera position from keyboard arrows
        {
            const float speed      = CameraPanSpeed * SecondsPerTick;
            int         directionX = (right ? 1 : 0) - (left ? 1 : 0);
            int         directionZ = (down ? 1 : 0) - (up ? 1 : 0);

            auto  dx        = directionX * speed;
            auto  dz        = directionZ * speed;
            auto& cameraPos = camera.getRawPosition();
            auto  newPos    = cameraConstraint.clamp(Vector2f(cameraPos.x + dx, cameraPos.z + dz));

            camera.setPosition(Vector3f(newPos.x, cameraPos.y, newPos.y));
        }

        // handle minimap dragging
        if (auto cursor = std::get_if<NormalCursorMode>(&cursorMode.getValue()); cursor != nullptr)
        {
            if (std::holds_alternative<NormalCursorMode::DraggingMinimapState>(cursor->state))
            {
                // ok, the cursor is dragging the minimap.
                // work out where the cursor is on the minimap,
                // convert that to the world, then set the camera's position to there
                // (clamped to map bounds)

                auto minimapToWorld = minimapToWorldMatrix(simulation.terrain, minimapRect);
                auto mousePos       = getMousePosition();
                auto worldPos
                    = minimapToWorld
                      * Vector3f(static_cast<float>(mousePos.x) + 0.5f, static_cast<float>(mousePos.y) + 0.5, 0.0f);
                auto newCameraPos = cameraConstraint.clamp(Vector2f(worldPos.x, worldPos.z));
                camera.setPosition(Vector3f(newCameraPos.x, camera.getRawPosition().y, newCameraPos.y));
            }
        }

        elementOfInterest = getElementOfInterest();

        if (auto buildCursor = std::get_if<BuildCursorMode>(&cursorMode.getValue());
            buildCursor != nullptr && isCursorOverWorld())
        {
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            auto intersect = simulation.intersectLineWithTerrain(ray.toLine());

            if (intersect)
            {
                auto        mc            = unitFactory.getAdHocMovementClass(buildCursor->unitType);
                const auto& unitType      = buildCursor->unitType;
                const auto& pos           = *intersect;
                auto        footprint     = unitFactory.getUnitFootprint(unitType);
                auto        footprintRect = computeFootprintRegion(pos, footprint.x, footprint.y);
                auto        isValid       = simulation.canBeBuiltAt(mc, footprintRect.x, footprintRect.y);
                hoverBuildInfo            = HoverBuildInfo{footprintRect, isValid};
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
            sceneContext.cursor->useCursor(CursorService::Type::normal);
        }
        else
        {
            match(
                cursorMode.getValue(),
                [&](const AttackCursorMode&) { sceneContext.cursor->useCursor(CursorService::Type::attack); },
                [&](const MoveCursorMode&) { sceneContext.cursor->useCursor(CursorService::Type::move); },
                [&](const BuildCursorMode&) { sceneContext.cursor->useCursor(CursorService::Type::normal); },
                [&](const RepairCursorMode&) { sceneContext.cursor->useCursor(CursorService::Type::repair); },
                [&](const NormalCursorMode&) {
                    match(
                        elementOfInterest,
                        [this](const UnitId hoveredUnitId) {
                            const auto& hoveredUnit = getUnit(hoveredUnitId);
                            if (std::any_of(selectedUnits.begin(),
                                            selectedUnits.end(),
                                            [&](const auto& id) { return getUnit(id).builder; })
                                && hoveredUnit.isOwnedBy(localPlayerId) && hoveredUnit.isDamaged())
                            {
                                sceneContext.cursor->useCursor(CursorService::Type::repair);
                            }
                            else if (hoveredUnit.isSelectableBy(localPlayerId))
                            {
                                sceneContext.cursor->useCursor(CursorService::Type::select);
                            }
                            else if (std::any_of(selectedUnits.begin(),
                                                 selectedUnits.end(),
                                                 [&](const auto& id) { return getUnit(id).canAttack; })
                                     && !hoveredUnit.isOwnedBy(localPlayerId))
                            {
                                sceneContext.cursor->useCursor(CursorService::Type::attack);
                            }
                            else
                            {
                                sceneContext.cursor->useCursor(CursorService::Type::normal);
                            }
                        },
                        [this](auto) { sceneContext.cursor->useCursor(CursorService::Type::normal); });
                });
        }

        auto maxRtt                   = std::clamp(gameNetworkService->getMaxAverageRttMillis(), 16.0f, 2000.0f);
        auto highCommandLatencyMillis = maxRtt + (maxRtt / 4.0f) + 200.0f;
        auto commandLatencyFrames     = static_cast<unsigned int>(highCommandLatencyMillis / 16.0f) + 1;
        auto targetCommandBufferSize  = commandLatencyFrames;

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
        for (unsigned int i = 0; i < simulation.players.size(); ++i)
        {
            PlayerId    id(i);
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
            nextPanel    = std::nullopt;
            attachOrdersMenuEventHandlers();
        }

        auto averageSceneTime = gameNetworkService->estimateAvergeSceneTime(sceneTime);

        // allow skipping sim frames every so often to get back down to average.
        // We tolerate X frames of drift in either direction to cope with noisiness in the
        // estimation.
        const SceneTime frameTolerance(3);
        const SceneTime frameCheckInterval(5);
        auto            highSceneTime = averageSceneTime + frameTolerance;
        auto lowSceneTime = averageSceneTime <= frameTolerance ? SceneTime{0} : averageSceneTime - frameTolerance;
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

    std::optional<UnitId> GameScene::spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        // TODO: if we failed to add the unit throw some warning
        auto unitId = simulation.tryAddUnit(
            unitFactory.createUnit(unitType, owner, simulation.getPlayer(owner).color, position));

        if (unitId)
        {
            const auto& unit = getUnit(*unitId);

            // set speed for metal extractors
            if (unit.extractsMetal != Metal(0))
            {
                auto footprint = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                auto metalValue
                    = simulation.metalGrid.accumulate(simulation.metalGrid.clipRegion(footprint), 0u, std::plus<>());
                unit.cobEnvironment->createThread("SetSpeed", {static_cast<int>(metalValue)});
            }

            // initialise local-player-specific UI data
            unitGuiInfos.insert_or_assign(
                *unitId, UnitGuiInfo{unit.builder ? UnitGuiInfo::Section::Build : UnitGuiInfo::Section::Orders, 0});
        }

        return unitId;
    }

    void GameScene::spawnCompletedUnit(const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        auto unitId = spawnUnit(unitType, owner, position);
        if (unitId)
        {
            auto& unit = getUnit(*unitId);
            // units start as unbuilt nanoframes,
            // we we need to convert it immediately into a completed unit.
            unit.finishBuilding();
        }
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        worldRenderService.getCamera().setPosition(newPosition);
    }

    const MapTerrain& GameScene::getTerrain() const { return simulation.terrain; }

    void GameScene::showObject(UnitId unitId, const std::string& name) { simulation.showObject(unitId, name); }

    void GameScene::hideObject(UnitId unitId, const std::string& name) { simulation.hideObject(unitId, name); }

    void GameScene::moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed)
    {
        simulation.moveObject(unitId, name, axis, position, speed);
    }

    void GameScene::moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position)
    {
        simulation.moveObjectNow(unitId, name, axis, position);
    }

    void GameScene::turnObject(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle, float speed)
    {
        simulation.turnObject(unitId, name, axis, angle, speed);
    }

    void GameScene::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, RadiansAngle angle)
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

    GameTime GameScene::getGameTime() const { return simulation.gameTime; }

    void GameScene::playSoundOnSelectChannel(const AudioService::SoundHandle& handle)
    {
        sceneContext.audioService->playSoundIfFree(handle, UnitSelectChannel);
    }

    void GameScene::playUnitSound(UnitId /*unitId*/, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a unit-specific channel group
        sceneContext.audioService->playSound(sound);
    }

    void GameScene::playSoundAt(const Vector3f& /*position*/, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a position-aware channel
        sceneContext.audioService->playSound(sound);
    }

    Matrix4f GameScene::worldToMinimapMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect)
    {
        auto view = Matrix4f::rotationToAxes(
            Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f));
        auto cabinet                  = Matrix4f::cabinetProjection(0.0f, 0.5f);
        auto orthographic             = Matrix4f::orthographicProjection(terrain.leftInWorldUnits(),
                                                             terrain.rightCutoffInWorldUnits(),
                                                             terrain.bottomCutoffInWorldUnits(),
                                                             terrain.topInWorldUnits(),
                                                             -1000.0f,
                                                             1000.0f);
        auto worldProjection          = orthographic * cabinet;
        auto minimapInverseProjection = Matrix4f::inverseOrthographicProjection(
            minimapRect.left(), minimapRect.right(), minimapRect.bottom(), minimapRect.top(), -1.0f, 1.0f);
        return minimapInverseProjection * worldProjection * view;
    }

    Matrix4f GameScene::minimapToWorldMatrix(const MapTerrain& terrain, const Rectangle2f& minimapRect)
    {
        auto view = Matrix4f::rotationToAxes(
            Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, -1.0f, 0.0f));
        auto inverseView            = view.transposed();
        auto inverseCabinet         = Matrix4f::cabinetProjection(0.0f, -0.5f);
        auto inverseOrthographic    = Matrix4f::inverseOrthographicProjection(terrain.leftInWorldUnits(),
                                                                           terrain.rightCutoffInWorldUnits(),
                                                                           terrain.bottomCutoffInWorldUnits(),
                                                                           terrain.topInWorldUnits(),
                                                                           -1000.0f,
                                                                           1000.0f);
        auto worldInverseProjection = inverseCabinet * inverseOrthographic;
        auto minimapProjection      = Matrix4f::orthographicProjection(
            minimapRect.left(), minimapRect.right(), minimapRect.bottom(), minimapRect.top(), -1.0f, 1.0f);
        return inverseView * worldInverseProjection * minimapProjection;
    }

    void GameScene::tryTickGame()
    {
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
        if (simulation.gameTime % GameTime(60) == GameTime(0))
        {
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
                    player.metalStalled                 = false;
                }
                else
                {
                    player.metalStalled = true;
                }

                player.previousDesiredMetalConsumptionBuffer = player.desiredMetalConsumptionBuffer;
                player.desiredMetalConsumptionBuffer         = Metal(0);

                if (player.energy > Energy(0))
                {
                    player.energy -= player.actualEnergyConsumptionBuffer;
                    player.actualEnergyConsumptionBuffer = Energy(0);
                    player.energyStalled                 = false;
                }
                else
                {
                    player.energyStalled = true;
                }

                player.previousDesiredEnergyConsumptionBuffer = player.desiredEnergyConsumptionBuffer;
                player.desiredEnergyConsumptionBuffer         = Energy(0);

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
                auto&       unit   = entry.second;

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
                            auto footprint  = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                            auto metalValue = simulation.metalGrid.accumulate(
                                simulation.metalGrid.clipRegion(footprint), 0u, std::plus<>());
                            simulation.addResourceDelta(
                                unitId, Energy(0), Metal(metalValue * unit.extractsMetal.value));
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
            auto  unitId = entry.first;
            auto& unit   = entry.second;

            unitBehaviorService.update(unitId);

            unit.mesh.update(SecondsPerTick);

            cobExecutionService.run(*this, simulation, unitId);
        }

        updateLasers();

        updateExplosions();

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
                delay(SceneTime(5 * 60), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
            },
            [&](const WinStatusDraw&) {
                delay(SceneTime(5 * 60), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
            },
            [&](const WinStatusUndecided&) {
                // do nothing, game still in progress
            });

        deleteDeadUnits();
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
                auto minimapPos        = worldToMinimap * unit.position;
                minimapPos.x           = std::floor(minimapPos.x);
                minimapPos.y           = std::floor(minimapPos.y);
                auto        ownerId    = unit.owner;
                auto        colorIndex = getPlayer(ownerId).color;
                const auto& sprite     = *minimapDots->sprites[colorIndex.value];
                auto        bounds     = sprite.bounds;

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
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            return getFirstCollidingUnit(ray);
        }

        return std::nullopt;
    }

    std::optional<Vector3f> GameScene::getMouseTerrainCoordinate() const
    {
        if (isCursorOverMinimap())
        {
            auto transform = minimapToWorldMatrix(simulation.terrain, minimapRect);
            auto mousePos  = getMousePosition();
            auto mouseX    = static_cast<float>(mousePos.x) + 0.5f;
            auto mouseY    = static_cast<float>(mousePos.y) + 0.5f;

            auto  startPoint = transform * Vector3f(mouseX, mouseY, -1.0f);
            auto  endPoint   = transform * Vector3f(mouseX, mouseY, 1.0f);
            auto  direction  = endPoint - startPoint;
            Ray3f ray(startPoint, direction);
            return simulation.intersectLineWithTerrain(ray.toLine());
        }

        if (isCursorOverWorld())
        {
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            return simulation.intersectLineWithTerrain(ray.toLine());
        }

        return std::nullopt;
    }

    std::variant<std::monostate, Vector3f, UnitId> GameScene::getElementOfInterest() const
    {
        if (auto unitId = getUnitUnderCursor(); unitId.has_value())
        {
            return *unitId;
        }
        if (auto mousePos = getMouseTerrainCoordinate(); mousePos.has_value())
        {
            return *mousePos;
        }
        return {};
    }


    Vector2f GameScene::screenToWorldClipSpace(Point p) const
    {
        return worldViewport.toClipSpace(sceneContext.viewportService->toOtherViewport(worldViewport, p));
    }

    bool GameScene::isCursorOverMinimap() const
    {
        auto mousePos = getMousePosition();
        return minimapRect.contains(mousePos.x, mousePos.y);
    }

    bool GameScene::isCursorOverWorld() const { return worldViewport.contains(getMousePosition()); }

    Point GameScene::getMousePosition() const
    {
        int x;
        int y;
        sceneContext.sdl->getMouseState(&x, &y);
        return Point(x, y);
    }

    std::optional<UnitId> GameScene::getFirstCollidingUnit(const Ray3f& ray) const
    {
        return simulation.getFirstCollidingUnit(ray);
    }

    void GameScene::localPlayerIssueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto kind = PlayerUnitCommand::IssueOrder::IssueKind::Immediate;
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::IssueOrder(order, kind)));

        if (std::holds_alternative<BuildOrder>(order))
        {
            if (sounds.okToBuild)
            {
                playSoundOnSelectChannel(*sounds.okToBuild);
            }
        }
        else
        {
            const auto& unit = getUnit(unitId);
            if (unit.okSound)
            {
                playSoundOnSelectChannel(*(unit.okSound));
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
                playSoundOnSelectChannel(*sounds.okToBuild);
            }
        }
    }

    void GameScene::localPlayerStopUnit(UnitId unitId)
    {
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::Stop()));

        const auto& unit = getUnit(unitId);
        if (unit.okSound)
        {
            playSoundOnSelectChannel(*(unit.okSound));
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
        localPlayerCommandBuffer.push_back(
            PlayerUnitCommand(unitId, PlayerUnitCommand::ModifyBuildQueue{count, unitType}));

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

    bool GameScene::isShiftDown() const { return leftShiftDown || rightShiftDown; }

    Unit& GameScene::getUnit(UnitId id) { return simulation.getUnit(id); }

    const Unit& GameScene::getUnit(UnitId id) const { return simulation.getUnit(id); }

    std::optional<std::reference_wrapper<Unit>> GameScene::tryGetUnit(UnitId id) { return simulation.tryGetUnit(id); }

    std::optional<std::reference_wrapper<const Unit>> GameScene::tryGetUnit(UnitId id) const
    {
        return simulation.tryGetUnit(id);
    }

    const GamePlayerInfo& GameScene::getPlayer(PlayerId player) const { return simulation.getPlayer(player); }

    DiscreteRect
    GameScene::computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const
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

    GameSimulation& GameScene::getSimulation() { return simulation; }

    const GameSimulation& GameScene::getSimulation() const { return simulation; }

    bool GameScene::isEnemy(UnitId id) const
    {
        // TODO: consider allies/teams here
        return !getUnit(id).isOwnedBy(localPlayerId);
    }

    void GameScene::updateLasers()
    {
        auto gameTime = getGameTime();
        for (auto& laser : simulation.lasers)
        {
            if (!laser)
            {
                continue;
            }

            laser->position += laser->velocity;

            // emit smoke trail
            if (laser->smokeTrail)
            {
                if (gameTime > laser->lastSmoke + *laser->smokeTrail)
                {
                    createLightSmoke(laser->position);
                    laser->lastSmoke = gameTime;
                }
            }

            // test collision with terrain
            auto terrainHeight = simulation.terrain.getHeightAt(laser->position.x, laser->position.z);
            auto seaLevel      = simulation.terrain.getSeaLevel();

            // test collision with sea
            // FIXME: waterweapons should be allowed in water
            if (seaLevel > terrainHeight && laser->position.y <= seaLevel)
            {
                doLaserImpact(laser, ImpactType::Water);
            }
            else if (laser->position.y <= terrainHeight)
            {
                doLaserImpact(laser, ImpactType::Normal);
            }
            else
            {
                // detect collision with something's footprint
                auto heightMapPos = simulation.terrain.worldToHeightmapCoordinate(laser->position);
                auto cellValue    = simulation.occupiedGrid.tryGet(heightMapPos);
                if (cellValue)
                {
                    auto collides = laserCollides(simulation, *laser, cellValue->get());
                    if (collides)
                    {
                        doLaserImpact(laser, ImpactType::Normal);
                    }
                }
            }

            // TODO: detect collision between a laser and the world boundary
        }
    }

    void GameScene::updateExplosions()
    {
        for (auto& exp : simulation.explosions)
        {
            if (!exp)
            {
                continue;
            }

            if (exp->isFinished(simulation.gameTime))
            {
                exp = std::nullopt;
                continue;
            }

            if (exp->floats)
            {
                // TODO: drift with the wind
                exp->position.y += 0.5f;
            }
        }
    }

    void GameScene::doLaserImpact(std::optional<LaserProjectile>& laser, ImpactType impactType)
    {
        switch (impactType)
        {
            case ImpactType::Normal:
            {
                if (laser->soundHit)
                {
                    playSoundAt(laser->position, *laser->soundHit);
                }
                if (laser->explosion)
                {
                    simulation.spawnExplosion(laser->position, *laser->explosion);
                }
                if (laser->endSmoke)
                {
                    createLightSmoke(laser->position);
                }
                break;
            }
            case ImpactType::Water:
            {
                if (laser->soundWater)
                {
                    playSoundAt(laser->position, *laser->soundWater);
                }
                if (laser->waterExplosion)
                {
                    simulation.spawnExplosion(laser->position, *laser->waterExplosion);
                }
                break;
            }
        }

        applyDamageInRadius(laser->position, laser->damageRadius, *laser);

        laser = std::nullopt;
    }

    void GameScene::applyDamageInRadius(const Vector3f& position, float radius, const LaserProjectile& laser)
    {
        auto minX = position.x - radius;
        auto maxX = position.x + radius;
        auto minZ = position.z - radius;
        auto maxZ = position.z + radius;

        auto minPoint = simulation.terrain.worldToHeightmapCoordinate(Vector3f(minX, position.y, minZ));
        auto maxPoint = simulation.terrain.worldToHeightmapCoordinate(Vector3f(maxX, position.y, maxZ));
        auto minCell  = simulation.terrain.getHeightMap().clampToCoords(minPoint);
        auto maxCell  = simulation.terrain.getHeightMap().clampToCoords(maxPoint);

        assert(minCell.x <= maxCell.x);
        assert(minCell.y <= maxCell.y);

        auto radiusSquared = radius * radius;

        std::unordered_set<UnitId> seenUnits;

        auto region = GridRegion::fromCoordinates(minCell, maxCell);

        // for each cell
        region.forEach([&](const auto& coords) {
            // check if it's in range
            auto        cellCenter = simulation.terrain.heightmapIndexToWorldCenter(coords.x, coords.y);
            Rectangle2f cellRectangle(Vector2f(cellCenter.x, cellCenter.z),
                                      Vector2f(MapTerrain::HeightTileWidthInWorldUnits / 2.0f,
                                               MapTerrain::HeightTileHeightInWorldUnits / 2.0f));
            auto        cellDistanceSquared = cellRectangle.distanceSquared(Vector2f(position.x, position.z));
            if (cellDistanceSquared > radiusSquared)
            {
                return;
            }

            // check if a unit (or feature) is there
            auto occupiedType = simulation.occupiedGrid.get(coords);
            auto u            = match(
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
            auto damageScale  = std::clamp(1.0f - (std::sqrt(unitDistanceSquared) / radius), 0.0f, 1.0f);
            auto rawDamage    = laser.getDamage(unit.unitType);
            auto scaledDamage = static_cast<unsigned int>(static_cast<float>(rawDamage) * damageScale);
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
        simulation.spawnSmoke(position, sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 1"));
    }

    void GameScene::activateUnit(UnitId unitId)
    {
        auto unit = tryGetUnit(unitId);
        if (unit)
        {
            unit->get().activate();

            if (unit->get().activateSound)
            {
                playUnitSound(unitId, *unit->get().activateSound);
            }

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

            if (unit->get().deactivateSound)
            {
                playUnitSound(unitId, *unit->get().deactivateSound);
            }

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

    void GameScene::setBuildStance(UnitId unitId, bool value) { getUnit(unitId).inBuildStance = value; }

    void GameScene::setYardOpen(UnitId unitId, bool value) { simulation.trySetYardOpen(unitId, value); }

    void GameScene::setBuggerOff(UnitId unitId, bool value)
    {
        if (value)
        {
            simulation.emitBuggerOff(unitId);
        }
    }

    void GameScene::deleteDeadUnits()
    {
        for (auto it = simulation.units.begin(); it != simulation.units.end();)
        {
            const auto& unit = it->second;
            if (unit.isDead())
            {
                deselectUnit(it->first);

                if (auto hoveredUnit = std::get_if<UnitId>(&elementOfInterest); *hoveredUnit == it->first)
                {
                    hoveredUnit = {};
                }

                auto footprintRect   = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                auto footprintRegion = simulation.occupiedGrid.tryToRegion(footprintRect);
                assert(!!footprintRegion);
                if (unit.isMobile)
                {
                    simulation.occupiedGrid.forEach(*footprintRegion,
                                                    [](auto& cell) { cell.occupiedType = OccupiedNone(); });
                }
                else
                {
                    simulation.occupiedGrid.forEach(*footprintRegion, [&](auto& cell) {
                        if (cell.buildingCell && cell.buildingCell->unit == it->first)
                        {
                            cell.buildingCell = BuildingOccupiedCell();
                        }
                    });
                }


                unitGuiInfos.erase(it->first);
                it = simulation.units.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    BoundingBox3f GameScene::createBoundingBox(const Unit& unit) const
    {
        auto footprint = simulation.computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        auto min       = Vector3f(footprint.x, unit.position.y, footprint.y);
        auto max
            = Vector3f(footprint.x + footprint.width, unit.position.y + unit.height, footprint.y + footprint.height);
        auto worldMin = simulation.terrain.heightmapToWorldSpace(min);
        auto worldMax = simulation.terrain.heightmapToWorldSpace(max);
        return BoundingBox3f::fromMinMax(worldMin, worldMax);
    }

    void GameScene::killUnit(UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);

        unit.markAsDead();

        // TODO: spawn debris particles, corpse
        if (unit.explosionWeapon)
        {
            auto impactType
                = unit.position.y < simulation.terrain.getSeaLevel() ? ImpactType::Water : ImpactType::Normal;
            std::optional<LaserProjectile> projectile = simulation.createProjectileFromWeapon(
                unit.owner, *unit.explosionWeapon, unit.position, Vector3f(0.0f, -1.0f, 0.0f));
            doLaserImpact(projectile, impactType);
        }
    }

    void GameScene::quietlyKillUnit(UnitId unitId)
    {
        auto& unit = simulation.getUnit(unitId);

        unit.markAsDead();
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
        for (const auto& p : commands)
        {
            PlayerCommandDispatcher dispatcher(this, p.first);
            for (const auto& c : p.second)
            {
                std::visit(dispatcher, c);
            }
        }
    }

    void GameScene::attachOrdersMenuEventHandlers()
    {
        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "ATTACK"))
        {
            p->get().addSubscription(cursorMode.subscribe(
                [& p = p->get()](const auto& v) { p.setToggledOn(std::holds_alternative<AttackCursorMode>(v)); }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "MOVE"))
        {
            p->get().addSubscription(cursorMode.subscribe(
                [& p = p->get()](const auto& v) { p.setToggledOn(std::holds_alternative<MoveCursorMode>(v)); }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "FIREORD"))
        {
            p->get().addSubscription(fireOrders.subscribe([& p = p->get()](const auto& v) {
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
            p->get().addSubscription(onOff.subscribe([& p = p->get()](const auto& v) { p.setStage(v ? 1 : 0); }));
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
        if (matchesWithSidePrefix("REPAIR", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (std::holds_alternative<RepairCursorMode>(cursorMode.getValue()))
            {
                cursorMode.next(NormalCursorMode());
            }
            else
            {
                cursorMode.next(RepairCursorMode());
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
                // FIXME: should set all to a consistent single fire order rather than
                // advancing all
                auto& u             = getUnit(selectedUnit);
                auto  newFireOrders = nextFireOrders(u.fireOrders);
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
                auto& u        = getUnit(selectedUnit);
                auto  newOnOff = !u.activated;
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

                const auto& unit         = getUnit(*selectedUnit);
                auto&       guiInfo      = unitGuiInfos.at(*selectedUnit);
                auto        pages        = unitFactory.getBuildPageCount(unit.unitType);
                guiInfo.currentBuildPage = (guiInfo.currentBuildPage + 1) % pages;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1),
                                                  *buildPanelDefinition,
                                                  unit.getBuildQueueTotals()));
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

                const auto& unit    = getUnit(*selectedUnit);
                auto&       guiInfo = unitGuiInfos.at(*selectedUnit);
                auto        pages   = unitFactory.getBuildPageCount(unit.unitType);
                assert(pages != 0);
                guiInfo.currentBuildPage = guiInfo.currentBuildPage == 0 ? pages - 1 : guiInfo.currentBuildPage - 1;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1),
                                                  *buildPanelDefinition,
                                                  unit.getBuildQueueTotals()));
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

                const auto& unit    = getUnit(*selectedUnit);
                auto&       guiInfo = unitGuiInfos.at(*selectedUnit);
                guiInfo.section     = UnitGuiInfo::Section::Build;

                auto buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
                if (buildPanelDefinition)
                {
                    setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1),
                                                  *buildPanelDefinition,
                                                  unit.getBuildQueueTotals()));
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

                auto& guiInfo   = unitGuiInfos.at(*selectedUnit);
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
        return selectedUnits.size() == 1 ? std::make_optional(*selectedUnits.begin()) : std::nullopt;
    }

    void GameScene::selectUnitsInBandbox(const DiscreteRect& box)
    {
        const auto&         camera    = worldRenderService.getCamera();
        const auto          cameraPos = camera.getPosition();
        auto                cameraBox = box.translate(-cameraPos.x, -cameraPos.z);
        const auto&         matrix    = camera.getViewProjectionMatrix();
        std::vector<UnitId> units;

        for (const auto& e : simulation.units)
        {
            if (!e.second.isSelectableBy(localPlayerId))
            {
                continue;
            }

            const auto& worldPos    = e.second.position;
            auto        clipPos     = matrix * worldPos;
            Point       viewportPos = worldViewport.toViewportSpace(clipPos.x, clipPos.y);
            if (!cameraBox.contains(viewportPos))
            {
                continue;
            }

            units.emplace_back(e.first);
        }

        replaceUnitSelection(units);
    }

    void GameScene::replaceUnitSelection(const UnitId& unitId)
    {
        selectedUnits.clear();
        selectedUnits.insert(unitId);

        const auto& unit           = getUnit(unitId);
        const auto& selectionSound = unit.selectionSound;
        if (selectionSound)
        {
            playSoundOnSelectChannel(*selectionSound);
        }

        onSelectedUnitsChanged();
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

    void GameScene::replaceUnitSelection(const std::vector<UnitId>& units)
    {
        selectedUnits.clear();
        for (const auto& unitId : units)
        {
            selectedUnits.insert(unitId);
        }

        if (units.size() == 1)
        {
            const auto& unit           = getUnit(units.front());
            const auto& selectionSound = unit.selectionSound;
            if (selectionSound)
            {
                playSoundOnSelectChannel(*selectionSound);
            }
        }
        else if (units.size() > 0)
        {
            playSoundOnSelectChannel(*sounds.selectMultipleUnits);
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

            const auto& guiInfo              = getGuiInfo(*unitId);
            auto        buildPanelDefinition = unitFactory.getBuilderGui(unit.unitType, guiInfo.currentBuildPage);
            if (guiInfo.section == UnitGuiInfo::Section::Build && buildPanelDefinition)
            {
                setNextPanel(createBuildPanel(unit.unitType + std::to_string(guiInfo.currentBuildPage + 1),
                                              *buildPanelDefinition,
                                              unit.getBuildQueueTotals()));
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

    void GameScene::setNextPanel(std::unique_ptr<UiPanel>&& panel) { nextPanel = std::move(panel); }

    void GameScene::refreshBuildGuiTotal(UnitId unitId, const std::string& unitType)
    {
        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit == unitId)
        {
            const auto& unit   = getUnit(*selectedUnit);
            auto        total  = unit.getBuildQueueTotal(unitType) + getUnconfirmedBuildQueueCount(unitId, unitType);
            auto        button = currentPanel->find<UiStagedButton>(unitType);
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

    std::unique_ptr<UiPanel> GameScene::createBuildPanel(const std::string&           guiName,
                                                         const std::vector<GuiEntry>& buildPanelDefinition,
                                                         const std::unordered_map<std::string, int>& totals)
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
}
