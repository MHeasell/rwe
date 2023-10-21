#include "GameScene.h"
#include <algorithm>
#include <boost/range/adaptor/map.hpp>
#include <fstream>
#include <functional>
#include <rwe/CroppedViewport.h>
#include <rwe/Mesh.h>
#include <rwe/camera_util.h>
#include <rwe/game/GameScene_util.h>
#include <rwe/game/dump_util.h>
#include <rwe/game/matrix_util.h>
#include <rwe/resource_io.h>
#include <rwe/sim/SimTicksPerSecond.h>
#include <rwe/ui/UiStagedButton.h>
#include <rwe/util/Index.h>
#include <rwe/util/match.h>
#include <spdlog/spdlog.h>

namespace rwe
{
    bool isValidUnitType(const GameSimulation& simulation, const std::string& unitType)
    {
        return simulation.unitDefinitions.find(unitType) != simulation.unitDefinitions.end();
    }

    bool featureCanBeReclaimed(const GameSimulation& sim, FeatureId featureId)
    {
        const auto& featureState = sim.getFeature(featureId);
        const auto& def = sim.getFeatureDefinition(featureState.featureName);
        return def.reclaimable;
    }

    std::optional<std::reference_wrapper<const std::vector<GuiEntry>>> getBuilderGui(const BuilderGuisDatabase& db, const std::string& unitType, unsigned int page)
    {
        const auto& pages = db.tryGetBuilderGui(unitType);
        if (!pages)
        {
            return std::nullopt;
        }

        const auto& unwrappedPages = pages->get();

        if (page >= unwrappedPages.size())
        {
            return std::nullopt;
        }

        return unwrappedPages[page];
    }

    /** If the unit has no build gui, this will be zero. */
    unsigned int getBuildPageCount(const BuilderGuisDatabase& db, const std::string& unitType)
    {
        const auto& pages = db.tryGetBuilderGui(unitType);
        if (!pages)
        {
            return 0;
        }

        return pages->get().size();
    }

    bool unitCanAttack(const GameSimulation& sim, UnitId unitId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.canAttack;
    }

    bool unitCanMove(const GameSimulation& sim, UnitId unitId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.canMove;
    }

    bool unitCanGuard(const GameSimulation& sim, UnitId unitId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.canGuard;
    }

    bool unitIsBuilder(const GameSimulation& sim, UnitId unitId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.builder;
    }

    bool unitIsBuilder(const GameSimulation& sim, std::optional<UnitId> singleSelectedUnit)
    {
        if (!singleSelectedUnit)
        {
            return false;
        }
        const auto& unit = sim.getUnitState(*singleSelectedUnit);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.builder;
    }

    bool unitIsBeingBuilt(const GameSimulation& sim, UnitId unitId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unit.isBeingBuilt(unitDefinition);
    }

    bool unitIsSelectableBy(const GameSimulation& sim, UnitId unitId, PlayerId playerId)
    {
        const auto& unit = sim.getUnitState(unitId);
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unit.isSelectableBy(unitDefinition, playerId);
    }

    bool unitIsOwnedByPlayerAndIsBuilder(const GameSimulation& sim, PlayerId playerId, std::optional<UnitId> singleSelectedUnit)
    {
        if (!singleSelectedUnit)
        {
            return false;
        }
        const auto& unit = sim.getUnitState(*singleSelectedUnit);
        if (!unit.isOwnedBy(playerId))
        {
            return false;
        }
        const auto& unitDefinition = sim.unitDefinitions.at(unit.unitType);
        return unitDefinition.builder;
    }

    bool shouldShowAllBuildBoxes(const GameSimulation& sim, PlayerId localPlayerId, std::optional<UnitId> singleSelectedUnit, std::optional<UnitId> hoveredUnit)
    {
        return unitIsBuilder(sim, singleSelectedUnit) || unitIsOwnedByPlayerAndIsBuilder(sim, localPlayerId, hoveredUnit);
    }

    Line3x<SimScalar> floatToSimLine(const Line3f& line)
    {
        return Line3x<SimScalar>(floatToSimVector(line.start), floatToSimVector(line.end));
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
        GameMediaDatabase&& meshDatabase,
        const GameCameraState& cameraState,
        SharedTextureHandle unitTextureAtlas,
        std::vector<SharedTextureHandle>&& unitTeamTextureAtlases,
        GameSimulation&& simulation,
        MapTerrainGraphics&& terrainGraphics,
        BuilderGuisDatabase&& builderGuisDatabase,
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
          gameMediaDatabase(std::move(meshDatabase)),
          unitTextureAtlas(unitTextureAtlas),
          unitTeamTextureAtlases(std::move(unitTeamTextureAtlases)),
          worldUiRenderService(this->sceneContext.graphics, this->sceneContext.shaders, &this->worldViewport),
          chromeUiRenderService(this->sceneContext.graphics, this->sceneContext.shaders, this->sceneContext.viewport),
          simulation(std::move(simulation)),
          terrainGraphics(std::move(terrainGraphics)),
          builderGuisDatabase(std::move(builderGuisDatabase)),
          gameNetworkService(std::move(gameNetworkService)),
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
            auto rectWidth = localPlayer.maxEnergy == Energy(0) ? 0 : (rect.width * std::max(Energy(0), localPlayer.energy).value) / localPlayer.maxEnergy.value;
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
            auto rectWidth = localPlayer.maxMetal == Metal(0) ? 0 : (rect.width * std::max(Metal(0), localPlayer.metal).value) / localPlayer.maxMetal.value;
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
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            if (logos)
            {
                const auto& rect = localSideData.logo2.toDiscreteRect();
                const auto& color = *(*logos)->sprites.at(getPlayer(unit.owner).color.value);
                chromeUiRenderService.drawSpriteAbs(rect.x, extraBottom + rect.y, rect.width, rect.height, color);
            }

            {
                const auto& rect = localSideData.unitName;
                const auto& playerName = getPlayer(unit.owner).name;
                const auto& text = unitDefinition.showPlayerName && playerName ? *playerName : unitDefinition.unitName;
                chromeUiRenderService.drawTextCenteredX(rect.x1, extraBottom + rect.y1, text, *guiFont);
            }

            if (unit.isOwnedBy(localPlayerId) || !unitDefinition.hideDamage)
            {
                const auto& rect = localSideData.damageBar.toDiscreteRect();
                chromeUiRenderService.drawHealthBar2(rect.x, extraBottom + rect.y, rect.width, rect.height, static_cast<float>(unit.hitPoints) / static_cast<float>(unitDefinition.maxHitPoints));
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
        else if (hoveredFeature)
        {
            const auto& feature = simulation.getFeature(*hoveredFeature);
            const auto& featureDefinition = simulation.getFeatureDefinition(feature.featureName);
            const auto& featureMediaInfo = gameMediaDatabase.getFeature(feature.featureName);

            {
                const auto& rect = localSideData._name;
                auto text = featureMediaInfo.description;
                if (featureDefinition.reclaimable)
                {
                    text += " ";
                    if (featureDefinition.metal > 0)
                    {
                        text += " M:" + formatResource(Metal(featureDefinition.metal));
                    }

                    if (featureDefinition.energy > 0)
                    {
                        text += " E:" + formatResource(Energy(featureDefinition.energy));
                    }
                }
                chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont);
            }
        }
        else if (auto hoveredBuildButtonUnitType = getUnitBuildButtonUnderCursor(); hoveredBuildButtonUnitType)
        {
            const auto& unitDefinition = simulation.unitDefinitions.at(*hoveredBuildButtonUnitType);

            {
                const auto& rect = localSideData._name;
                auto text = unitDefinition.unitName + "  M:" + formatResource(unitDefinition.buildCostMetal) + " E:" + formatResource(unitDefinition.buildCostEnergy);
                chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, text, *guiFont);
            }

            {
                const auto& rect = localSideData.description;
                chromeUiRenderService.drawText(rect.x1, extraBottom + rect.y1, unitDefinition.unitDescription, *guiFont);
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

    void GameScene::renderBuildBoxes(const UnitState& unit, const Color& color)
    {
        auto worldToUi = worldUiRenderService.getInverseViewProjectionMatrix()
            * computeViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height());
        for (const auto& order : unit.orders)
        {
            if (const auto buildOrder = std::get_if<BuildOrder>(&order))
            {
                const auto& unitType = buildOrder->unitType;
                const auto& unitDefinition = simulation.unitDefinitions.at(unitType);
                auto mc = simulation.getAdHocMovementClass(unitDefinition.movementCollisionInfo);
                auto footprintRect = simulation.computeFootprintRegion(buildOrder->position, unitDefinition.movementCollisionInfo);

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
                },
                [&](const ReclaimOrder& o) {
                    return match(
                        o.target,
                        [&](const UnitId& u) {
                            auto unitOption = tryGetUnit(u);
                            if (!unitOption)
                            {
                                return pos;
                            }
                            return unitOption->get().position;
                        },
                        [&](const FeatureId& f) {
                            auto featureOption = simulation.tryGetFeature(f);
                            if (!featureOption)
                            {
                                return pos;
                            }
                            return featureOption->get().position;
                        });
                });

            auto waypointIcon = match(
                order,
                [&](const BuildOrder&) { return std::optional<CursorType>(); },
                [&](const MoveOrder&) { return std::optional<CursorType>(CursorType::Move); },
                [&](const AttackOrder&) { return std::optional<CursorType>(CursorType::Attack); },
                [&](const BuggerOffOrder&) { return std::optional<CursorType>(); },
                [&](const CompleteBuildOrder&) { return std::optional<CursorType>(CursorType::Repair); },
                [&](const GuardOrder&) { return std::optional<CursorType>(CursorType::Guard); },
                [&](const ReclaimOrder&) { return std::optional<CursorType>(CursorType::Reclaim); });

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
                    [&](const GuardOrder&) { return true; },
                    [&](const ReclaimOrder&) { return true; });

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
        RenderService worldRenderService(sceneContext.graphics, sceneContext.shaders, &viewProjectionMatrix);

        sceneContext.graphics->disableDepthBuffer();

        worldRenderService.drawMapTerrain(terrainGraphics, worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));

        SpriteBatch flatFeatureBatch;
        SpriteBatch flatFeatureShadowBatch;
        for (const auto& f : simulation.features)
        {
            const auto& featureDefinition = simulation.getFeatureDefinition(f.second.featureName);
            if (!featureDefinition.isStanding())
            {
                drawFeature(gameMediaDatabase, f.second, featureDefinition, viewProjectionMatrix, flatFeatureBatch);
                drawFeatureShadow(gameMediaDatabase, f.second, featureDefinition, viewProjectionMatrix, flatFeatureShadowBatch);
            }
        }
        worldRenderService.drawSpriteBatch(flatFeatureShadowBatch);
        worldRenderService.drawSpriteBatch(flatFeatureBatch);

        ColoredMeshBatch squareParticlesBatch;
        for (const auto& particle : particles)
        {
            drawWakeParticle(gameMediaDatabase, simulation.gameTime, viewProjectionMatrix, particle, squareParticlesBatch);
        }
        worldRenderService.drawBatch(squareParticlesBatch, viewProjectionMatrix);

        ColoredMeshBatch terrainOverlayBatch;

        if (occupiedGridVisible)
        {
            drawOccupiedGrid(worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()), simulation.terrain, simulation.occupiedGrid, terrainOverlayBatch);
        }
        if (pathfindingVisualisationVisible)
        {
            drawPathfindingVisualisation(simulation.terrain, simulation.pathFindingService.lastPathDebugInfo, terrainOverlayBatch);
        }

        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && movementClassGridVisible)
        {
            const auto& unit = simulation.getUnitState(*selectedUnit);
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            match(
                unitDefinition.movementCollisionInfo,
                [&](const UnitDefinition::NamedMovementClass& c) {
                    const auto& grid = simulation.movementClassCollisionService.getGrid(c.movementClassId);
                    drawMovementClassCollisionGrid(simulation.terrain, grid, worldCameraState.getRoundedPosition(), worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()), terrainOverlayBatch);
                },
                [&](const auto&) {

                });
        }

        worldRenderService.drawBatch(terrainOverlayBatch, viewProjectionMatrix);

        auto interpolationFraction = static_cast<float>(millisecondsBuffer) / static_cast<float>(SimMillisecondsPerTick);
        ColoredMeshesBatch selectionRectBatch;
        for (const auto& selectedUnitId : selectedUnits)
        {
            const auto& unit = getUnit(selectedUnitId);
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            drawSelectionRect(gameMediaDatabase, viewProjectionMatrix, unit, unitDefinition, interpolationFraction, selectionRectBatch);
        }
        worldRenderService.drawLineLoopsBatch(selectionRectBatch);

        auto seaLevel = simulation.terrain.getSeaLevel();

        UnitShadowMeshBatch unitShadowMeshBatch;
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            const auto& modelDefinition = simulation.unitModelDefinitions.at(unitDefinition.objectName);

            auto groundHeight = simulation.terrain.getHeightAt(unit.position.x, unit.position.z);
            if (unitDefinition.floater || unitDefinition.canHover)
            {
                groundHeight = rweMax(groundHeight, seaLevel);
            }
            drawUnitShadow(gameMediaDatabase, viewProjectionMatrix, unit, unitDefinition, modelDefinition, interpolationFraction, simScalarToFloat(groundHeight), unitTextureAtlas.get(), unitTeamTextureAtlases, unitShadowMeshBatch);
        }
        for (const auto& feature : (simulation.features | boost::adaptors::map_values))
        {
            const auto& position = feature.position;
            auto groundHeight = simulation.terrain.getHeightAt(position.x, position.z);
            if (position.y >= seaLevel && groundHeight < seaLevel)
            {
                groundHeight = seaLevel;
            }

            drawFeatureMeshShadow(simulation.unitModelDefinitions, gameMediaDatabase, viewProjectionMatrix, feature, simScalarToFloat(groundHeight), unitTextureAtlas.get(), unitTeamTextureAtlases, unitShadowMeshBatch);
        }
        worldRenderService.drawUnitShadowMeshBatch(unitShadowMeshBatch);

        sceneContext.graphics->enableDepthBuffer();

        UnitMeshBatch unitMeshBatch;
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            const auto& unitModelDefinition = simulation.unitModelDefinitions.at(unitDefinition.objectName);
            drawUnit(gameMediaDatabase, viewProjectionMatrix, unit, unitDefinition, unitModelDefinition, getPlayer(unit.owner).color, interpolationFraction, unitTextureAtlas.get(), unitTeamTextureAtlases, unitMeshBatch);
        }
        for (const auto& feature : (simulation.features | boost::adaptors::map_values))
        {
            drawMeshFeature(simulation.unitModelDefinitions, gameMediaDatabase, viewProjectionMatrix, feature, unitTextureAtlas.get(), unitTeamTextureAtlases, unitMeshBatch);
        }
        worldRenderService.drawUnitMeshBatch(unitMeshBatch, simScalarToFloat(seaLevel), simulation.gameTime.value);

        ColoredMeshBatch lineProjectilesBatch;
        SpriteBatch spriteProjectilesBatch;
        UnitMeshBatch meshProjectilesBatch;
        drawProjectiles(simulation, gameMediaDatabase, viewProjectionMatrix, simulation.projectiles, simulation.gameTime, interpolationFraction, unitTextureAtlas.get(), unitTeamTextureAtlases, lineProjectilesBatch, spriteProjectilesBatch, meshProjectilesBatch);
        worldRenderService.drawBatch(lineProjectilesBatch, viewProjectionMatrix);
        worldRenderService.drawUnitMeshBatch(meshProjectilesBatch, simScalarToFloat(seaLevel), simulation.gameTime.value);
        worldRenderService.drawSpriteBatch(spriteProjectilesBatch);

        sceneContext.graphics->disableDepthWrites();

        SpriteBatch featureBatch;
        SpriteBatch featureShadowBatch;
        for (const auto& f : simulation.features)
        {
            const auto& featureDefinition = simulation.getFeatureDefinition(f.second.featureName);
            if (featureDefinition.isStanding())
            {
                drawFeature(gameMediaDatabase, f.second, featureDefinition, viewProjectionMatrix, featureBatch);
                drawFeatureShadow(gameMediaDatabase, f.second, featureDefinition, viewProjectionMatrix, featureShadowBatch);
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
                auto targetPositionOption = match(
                    std::get<0>(*nanolatheTarget),
                    [&](const UnitId& targetUnitId) -> std::optional<SimVector> {
                        auto targetUnitOption = tryGetUnit(targetUnitId);
                        if (!targetUnitOption)
                        {
                            return std::nullopt;
                        }
                        return targetUnitOption->get().position;
                    },
                    [&](const FeatureId& targetFeatureId) -> std::optional<SimVector> {
                        auto targetFeature = simulation.tryGetFeature(targetFeatureId);
                        if (!targetFeature)
                        {
                            return std::nullopt;
                        }
                        return targetFeature->get().position;
                    });

                if (targetPositionOption)
                {
                    switch (std::get<2>(*nanolatheTarget))
                    {
                        case UnitState::NanolatheDirection::Forward:
                            drawNanoLine(simVectorToFloat(std::get<1>(*nanolatheTarget)), simVectorToFloat(*targetPositionOption), nanoLinesBatch);
                            break;
                        case UnitState::NanolatheDirection::Reverse:
                            drawReverseNanoLine(simVectorToFloat(std::get<1>(*nanolatheTarget)), simVectorToFloat(*targetPositionOption), nanoLinesBatch);
                            break;
                        default:
                            throw std::logic_error("unhandled nanolathe direction");
                    }
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

        SpriteBatch spriteParticlesBatch;
        for (const auto& particle : particles)
        {
            drawSpriteParticle(gameMediaDatabase, simulation.gameTime, viewProjectionMatrix, particle, spriteParticlesBatch);
        }
        worldRenderService.drawSpriteBatch(spriteParticlesBatch);
        sceneContext.graphics->enableDepthTest();

        sceneContext.graphics->enableDepthWrites();

        // in-world UI/overlay rendering
        if (isShiftDown())
        {
            auto singleSelectedUnit = getSingleSelectedUnit();

            // if unit is a builder, show all other buildings being built
            if (shouldShowAllBuildBoxes(simulation, localPlayerId, singleSelectedUnit, hoveredUnit))
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
            for (const UnitState& unit : (simulation.units | boost::adaptors::map_values))
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

                const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);

                auto uiPos = worldUiRenderService.getInverseViewProjectionMatrix()
                    * viewProjectionMatrix
                    * simVectorToFloat(unit.position);
                worldUiRenderService.drawHealthBar(uiPos.x, uiPos.y, static_cast<float>(unit.hitPoints) / static_cast<float>(unitDefinition.maxHitPoints));
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

    const char* stateToString(const UnitBehaviorState& state)
    {
        return match(
            state,
            [&](const UnitBehaviorStateIdle&) {
                return "idle";
            },
            [&](const UnitBehaviorStateBuilding&) {
                return "building";
            },
            [&](const UnitBehaviorStateReclaiming&) {
                return "reclaiming";
            },
            [&](const UnitBehaviorStateCreatingUnit&) {
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

    void renderUnitInfoSection(const UnitState& unit)
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Unit Info"))
        {
            ImGui::LabelText("State", "%s", stateToString(unit.behaviourState));

            ImGui::LabelText("x", "%f", unit.position.x.value);
            ImGui::LabelText("y", "%f", unit.position.y.value);
            ImGui::LabelText("z", "%f", unit.position.z.value);
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("COB Scripts"))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Static Variables"))
            {
                for (Index i = 0; i < getSize(unit.cobEnvironment->_statics); ++i)
                {
                    ImGui::Text("%lld: %d", i, unit.cobEnvironment->_statics[i]);
                }
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Threads"))
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("All"))
                {

                    for (Index i = 0; i < getSize(unit.cobEnvironment->threads); ++i)
                    {
                        const auto& thread = *unit.cobEnvironment->threads[i];
                        ImGui::Text("%lld: %s (%u)", i, thread.name.c_str(), thread.signalMask);
                    }
                    ImGui::TreePop();
                }

                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Ready"))
                {
                    for (Index i = 0; i < getSize(unit.cobEnvironment->readyQueue); ++i)
                    {
                        ImGui::Text("%s", unit.cobEnvironment->readyQueue[i]->name.c_str());
                    }
                    ImGui::TreePop();
                }

                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Blocked"))
                {
                    for (Index i = 0; i < getSize(unit.cobEnvironment->blockedQueue); ++i)
                    {
                        const auto& pair = unit.cobEnvironment->blockedQueue[i];
                        ImGui::Text("%s, %s", pair.second->name.c_str(), blockedStatusToString(pair.first).c_str());
                    }
                    ImGui::TreePop();
                }

                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Sleeping"))
                {
                    for (Index i = 0; i < getSize(unit.cobEnvironment->sleepingQueue); ++i)
                    {
                        const auto& pair = unit.cobEnvironment->sleepingQueue[i];
                        ImGui::Text("%s, wake time: %d", pair.second->name.c_str(), pair.first.value);
                    }
                    ImGui::TreePop();
                }

                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode("Finished"))
                {
                    for (Index i = 0; i < getSize(unit.cobEnvironment->finishedQueue); ++i)
                    {
                        ImGui::Text("%s", unit.cobEnvironment->finishedQueue[i]->name.c_str());
                    }
                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }
        }
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
            text = toUpper(text);
            if (!text.empty() && isValidUnitType(simulation, text) && unitSpawnPlayer >= 0 && unitSpawnPlayer < getSize(simulation.players))
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

        if (ImGui::CollapsingHeader("Selected Unit"))
        {
            ImGui::Indent();
            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                const auto& unit = getUnit(*selectedUnit);
                renderUnitInfoSection(unit);
            }
            else
            {
                ImGui::Text("None");
            }
            ImGui::Unindent();
        }

        auto mouseTerrainCoordinate = getMouseTerrainCoordinate();

        if (mouseTerrainCoordinate)
        {
            ImGui::LabelText("mouse terrain x", "%f", mouseTerrainCoordinate->x.value);
            ImGui::LabelText("mouse terrain y", "%f", mouseTerrainCoordinate->y.value);
            ImGui::LabelText("mouse terrain z", "%f", mouseTerrainCoordinate->z.value);
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
        else if (keysym.sym == SDLK_F10)
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
                std::optional<UnitId> commanderUnitId;
                for (const auto& [unitId, unit] : simulation.units)
                {
                    const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
                    if (unitDefinition.commander && unit.isOwnedBy(localPlayerId))
                    {
                        selectAdditionalUnit(unitId);
                        // For multiple commanders, OTA selects all but always tracks only the last spawned (it won't cycle with repeated ctrl-c)
                        commanderUnitId = unitId;
                    }
                }

                if (commanderUnitId)
                {
                    startTrackInternal({*commanderUnitId});
                }
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
                [&](const ReclaimCursorMode&) {
                    for (const auto& selectedUnit : selectedUnits)
                    {
                        if (hoveredUnit)
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(selectedUnit, ReclaimOrder(*hoveredUnit));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(selectedUnit, ReclaimOrder(*hoveredUnit));
                                cursorMode.next(NormalCursorMode());
                            }
                        }
                        else if (hoveredFeature)
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
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
                                        if (const auto& u = getUnit(*hoveredUnit); u.isBeingBuilt(simulation.unitDefinitions.at(u.unitType)))
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
                [&](const ReclaimCursorMode&) {
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
                                    if (const auto& u = getUnit(*hoveredUnit); u.isBeingBuilt(simulation.unitDefinitions.at(u.unitType)))
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
                            else if (hoveredFeature && featureCanBeReclaimed(simulation, *hoveredFeature))
                            {
                                if (isShiftDown())
                                {
                                    localPlayerEnqueueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
                                }
                                else
                                {
                                    localPlayerIssueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
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
        else if (event.button == MouseButtonEvent::MouseButton::Middle)
        {
            cameraControlState = CameraControlStateMiddleMousePan{getMousePosition()};
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
                                if (hoveredUnit && getUnit(*hoveredUnit).isSelectableBy(simulation.unitDefinitions.at(getUnit(*hoveredUnit).unitType), localPlayerId))
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
                                        if (const auto& u = getUnit(*hoveredUnit); u.isBeingBuilt(simulation.unitDefinitions.at(u.unitType)))
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
                                else if (leftClickMode() && hoveredFeature && featureCanBeReclaimed(simulation, *hoveredFeature))
                                {
                                    for (const auto& selectedUnit : selectedUnits)
                                    {
                                        if (isShiftDown())
                                        {
                                            localPlayerEnqueueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
                                        }
                                        else
                                        {
                                            localPlayerIssueUnitOrder(selectedUnit, ReclaimOrder(*hoveredFeature));
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
        else if (event.button == MouseButtonEvent::MouseButton::Middle)
        {
            if (std::holds_alternative<CameraControlStateMiddleMousePan>(cameraControlState))
            {
                cameraControlState = CameraControlStateFree();
            }
        }
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

    void GameScene::onMouseMove(MouseMoveEvent event)
    {
        if (auto middleMousePanningState = std::get_if<CameraControlStateMiddleMousePan>(&cameraControlState); middleMousePanningState)
        {
            auto cameraConstraint = computeCameraConstraint(simulation.terrain, worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));

            const auto& cameraPos = worldCameraState.position;

            auto currentCursorPosition = Point(event.x, event.y);
            auto delta = currentCursorPosition - middleMousePanningState->previousCursorPosition;

            auto newCameraPos = cameraConstraint.clamp(Vector2f(cameraPos.x - delta.x, cameraPos.z - delta.y));
            worldCameraState.position = Vector3f(newCameraPos.x, cameraPos.y, newCameraPos.y);

            middleMousePanningState->previousCursorPosition = currentCursorPosition;
        }
        currentPanel->mouseMove(event);
    }

    void GameScene::onMouseWheel(MouseWheelEvent event)
    {
        currentPanel->mouseWheel(event);
    }

    void GameScene::update(int millisecondsElapsed)
    {
        millisecondsBuffer += millisecondsElapsed;

        auto cameraConstraint = computeCameraConstraint(simulation.terrain, worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));

        // update camera position from keyboard arrows
        {
            int directionX = (right ? 1 : 0) - (left ? 1 : 0);
            int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

            if (directionX || directionZ)
            {
                nudgeCamera(millisecondsElapsed, cameraConstraint, directionX, directionZ);
            }
        }

        // update camera position from edge scroll
        {
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
                nudgeCamera(millisecondsElapsed, cameraConstraint, directionX, directionZ);
            }
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
                auto mousePos = getMousePosition();
                auto worldPos = minimapToWorld * Vector3f(static_cast<float>(mousePos.x) + 0.5f, static_cast<float>(mousePos.y) + 0.5, 0.0f);

                relocateCamera(cameraConstraint, worldPos.x, worldPos.z);
            }
        }

        // handle tracking
        {
            // TODO (kwh) - tracking of projectiles not yet implemented. E.g. while tracking Bertha or Nuke Silo,
            // screen should follow a projectile until it hits, then return to the tracking group

            if (std::holds_alternative<CameraControlStateTrackingUnit>(cameraControlState) && trackedUnitId)
            {
                // get tracked unit position, or stop tracking if it's gone
                auto unit = tryGetUnit(*trackedUnitId);
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
        hoveredFeature = getFeatureUnderCursor();

        if (auto buildCursor = std::get_if<BuildCursorMode>(&cursorMode.getValue()); buildCursor != nullptr && isCursorOverWorld())
        {
            auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
            auto intersect = simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));

            if (intersect)
            {
                const auto& unitType = buildCursor->unitType;
                const auto& pos = *intersect;
                const auto& unitDefinition = simulation.unitDefinitions.at(unitType);
                auto mc = simulation.getAdHocMovementClass(unitDefinition.movementCollisionInfo);
                auto footprintRect = simulation.computeFootprintRegion(pos, unitDefinition.movementCollisionInfo);
                auto isValid = simulation.canBeBuiltAt(mc, unitDefinition.yardMap, unitDefinition.yardMapContainsGeo, footprintRect.x, footprintRect.y);
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
                [&](const ReclaimCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Reclaim);
                },
                [&](const BuildCursorMode&) {
                    sceneContext.cursor->useCursor(CursorType::Normal);
                },
                [&](const NormalCursorMode&) {
                    if (leftClickMode())
                    {
                        if (hoveredUnit && unitIsSelectableBy(simulation, *hoveredUnit, localPlayerId))
                        {
                            sceneContext.cursor->useCursor(CursorType::Select);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitCanAttack(simulation, id); })
                            && hoveredUnit && isEnemy(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Attack);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitIsBuilder(simulation, id); })
                            && hoveredUnit && unitIsBeingBuilt(simulation, *hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Repair);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitIsBuilder(simulation, id); }) && hoveredFeature && featureCanBeReclaimed(simulation, *hoveredFeature))
                        {
                            sceneContext.cursor->useCursor(CursorType::Reclaim);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitCanMove(simulation, id); }))
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
                        if (hoveredUnit && unitIsSelectableBy(simulation, *hoveredUnit, localPlayerId))
                        {
                            sceneContext.cursor->useCursor(CursorType::Select);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitCanAttack(simulation, id); })
                            && hoveredUnit && isEnemy(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Red);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitIsBuilder(simulation, id); })
                            && hoveredUnit && isFriendly(*hoveredUnit) && unitIsBeingBuilt(simulation, *hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Green);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitCanGuard(simulation, id); })
                            && hoveredUnit && isFriendly(*hoveredUnit))
                        {
                            sceneContext.cursor->useCursor(CursorType::Green);
                        }
                        else if (std::any_of(selectedUnits.begin(), selectedUnits.end(), [&](const auto& id) { return unitIsBuilder(simulation, id); }) && hoveredFeature && featureCanBeReclaimed(simulation, *hoveredFeature))
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
        return simulation.trySpawnUnit(unitType, owner, position, rotation);
    }

    std::optional<std::reference_wrapper<UnitState>> GameScene::spawnCompletedUnit(const std::string& unitType, PlayerId owner, const SimVector& position)
    {
        auto unitId = spawnUnit(unitType, owner, position, std::nullopt);
        if (unitId)
        {
            auto& unit = getUnit(*unitId);
            const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
            // units start as unbuilt nanoframes,
            // we we need to convert it immediately into a completed unit.
            unit.finishBuilding(unitDefinition);

            return unit;
        }

        return std::nullopt;
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        auto cameraConstraint = computeCameraConstraint(simulation.terrain, worldCameraState.scaleDimension(worldViewport.width()), worldCameraState.scaleDimension(worldViewport.height()));
        auto constrainedPosition = cameraConstraint.clamp(Vector2f(newPosition.x, newPosition.z));
        worldCameraState.position = Vector3f(constrainedPosition.x, newPosition.y, constrainedPosition.y);
    }

    const MapTerrain& GameScene::getTerrain() const
    {
        return simulation.terrain;
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

    std::optional<AudioService::SoundHandle> getSound(const GameSimulation& sim, const GameMediaDatabase& meshDb, const std::string& unitType, UnitSoundType soundType)
    {
        const auto& unitDefinition = sim.unitDefinitions.at(unitType);
        const auto& soundClass = meshDb.getSoundClassOrDefault(unitDefinition.soundCategory);
        const auto& soundId = getSoundName(soundClass, soundType);
        if (soundId)
        {
            return meshDb.tryGetSoundHandle(*soundId);
        }
        return std::nullopt;
    }

    void GameScene::playUnitNotificationSound(const PlayerId& playerId, const std::string& unitType, UnitSoundType soundType)
    {
        auto sound = getSound(simulation, gameMediaDatabase, unitType, soundType);
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

        const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(weaponType);
        if (weaponMediaInfo.soundStart)
        {
            auto sound = gameMediaDatabase.tryGetSoundHandle(*weaponMediaInfo.soundStart);
            if (sound)
            {
                playSoundAt(position, *sound);
            }
        }
    }

    void GameScene::playWeaponImpactSound(const Vector3f& position, const std::string& weaponType, ImpactType impactType)
    {
        const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(weaponType);
        switch (impactType)
        {
            case ImpactType::Normal:
            {
                if (weaponMediaInfo.soundHit)
                {
                    auto sound = gameMediaDatabase.tryGetSoundHandle(*weaponMediaInfo.soundHit);
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
                    auto sound = gameMediaDatabase.tryGetSoundHandle(*weaponMediaInfo.soundWater);
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
        const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(weaponType);

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

        processActions();

        processPlayerCommands(*playerCommands);

        simulation.tick();

        auto gameHash = simulation.computeHash();
        playerCommandService->pushHash(localPlayerId, gameHash);
        gameNetworkService->submitGameHash(gameHash);

        if (stateLogStream)
        {
            *stateLogStream << dumpJson(simulation) << std::endl;
        }

        processSimEvents();

        updateProjectiles();

        updateFlashes();

        updateParticles(gameMediaDatabase, simulation.gameTime, particles);

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

    std::optional<FeatureId> GameScene::getFeatureUnderCursor() const
    {
        if (!isCursorOverWorld())
        {
            return std::nullopt;
        }

        auto ray = screenToWorldRayUtil(computeInverseViewProjectionMatrix(worldCameraState, worldViewport.width(), worldViewport.height()), screenToWorldClipSpace(getMousePosition()));
        return getFirstCollidingFeature(ray);
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
        auto winnerIsMobile = false;
        auto bestDistance = std::numeric_limits<float>::infinity();
        std::optional<UnitId> it;

        for (const auto& entry : simulation.units)
        {
            const auto& unitDefinition = simulation.unitDefinitions.at(entry.second.unitType);
            auto selectionMesh = gameMediaDatabase.getSelectionCollisionMesh(unitDefinition.objectName);
            auto distance = selectionIntersect(entry.second, *selectionMesh.value(), ray);
            auto isMobile = unitDefinition.isMobile;
            if (distance && ((!winnerIsMobile && isMobile) || distance < bestDistance))
            {
                winnerIsMobile = isMobile;
                bestDistance = *distance;
                it = entry.first;
            }
        }

        return it;
    }

    std::optional<FeatureId> GameScene::getFirstCollidingFeature(const Ray3f& ray) const
    {
        auto intersect = simulation.intersectLineWithTerrain(floatToSimLine(ray.toLine()));
        if (!intersect)
        {
            return std::nullopt;
        }

        auto heightmapPosition = simulation.terrain.worldToHeightmapCoordinate(*intersect);

        auto cellContents = simulation.occupiedGrid.tryGet(heightmapPosition);
        if (!cellContents)
        {
            return std::nullopt;
        }

        return cellContents->get().featureId;
    }

    std::optional<float> GameScene::selectionIntersect(const UnitState& unit, const CollisionMesh& mesh, const Ray3f& ray) const
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
            auto handle = getSound(simulation, gameMediaDatabase, unit.unitType, UnitSoundType::Ok1);
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
        auto handle = getSound(simulation, gameMediaDatabase, unit.unitType, UnitSoundType::Ok1);
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
        // sort selection by unit id so repeated 'T' keydown cycles through all units in a group consistently
        std::vector<UnitId> unitIds;
        for (const auto& u : selectedUnits)
        {
            unitIds.push_back(u);
        }
        std::sort(unitIds.begin(), unitIds.end());

        startTrackInternal(unitIds);
    }

    void GameScene::startTrackInternal(const std::vector<UnitId>& unitIds)
    {
        // Only allow tracking in free camera mode or if we are already tracking.
        auto canStartTracking = match(
            cameraControlState,
            [&](const CameraControlStateFree&) {
                return true;
            },
            [&](const CameraControlStateTrackingUnit&) {
                return true;
            },
            [&](const CameraControlStateMiddleMousePan&) {
                return false;
            });

        if (!canStartTracking)
        {
            return;
        }

        // If 'T' is pressed and no units are selected, stop tracking.
        if (unitIds.empty())
        {
            cameraControlState = CameraControlStateFree();
            return;
        }

        // If already tracking, check if currently tracked unit is in this selection. If it is, select the next id in the group.
        if (trackedUnitId)
        {
            auto it = std::find(unitIds.begin(), unitIds.end(), trackedUnitId);
            if (it != unitIds.end() && ++it != unitIds.end())
            {
                trackedUnitId = *it;
            }
            else
            {
                trackedUnitId = unitIds[0];
            }
        }
        else
        {
            trackedUnitId = unitIds[0];
        }

        cameraControlState = CameraControlStateTrackingUnit();
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

    UnitState& GameScene::getUnit(UnitId id)
    {
        return simulation.getUnitState(id);
    }

    const UnitState& GameScene::getUnit(UnitId id) const
    {
        return simulation.getUnitState(id);
    }

    std::optional<std::reference_wrapper<UnitState>> GameScene::tryGetUnit(UnitId id)
    {
        return simulation.tryGetUnitState(id);
    }

    std::optional<std::reference_wrapper<const UnitState>> GameScene::tryGetUnit(UnitId id) const
    {
        return simulation.tryGetUnitState(id);
    }

    GamePlayerInfo& GameScene::getPlayer(PlayerId player)
    {
        return simulation.getPlayer(player);
    }

    const GamePlayerInfo& GameScene::getPlayer(PlayerId player) const
    {
        return simulation.getPlayer(player);
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
        for (auto& [projectileId, projectile] : simulation.projectiles)
        {
            const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(projectile.weaponType);
            auto& renderInfo = projectileRenderInfos[projectileId];

            // emit smoke trail
            if (weaponMediaInfo.smokeTrail)
            {
                auto gameTime = getGameTime();
                if (gameTime > projectile.lastSmoke + *weaponMediaInfo.smokeTrail)
                {
                    createLightSmoke(simVectorToFloat(projectile.position));
                    projectile.lastSmoke = gameTime;
                }
            }
        }
    }

    void GameScene::processSimEvents()
    {
        for (const auto& event : simulation.events)
        {
            match(
                event,
                [&](const FireWeaponEvent& e) {
                    const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(e.weaponType);

                    if (e.shotNumber == 0 || weaponMediaInfo.soundTrigger)
                    {
                        playWeaponStartSound(simVectorToFloat(e.firePoint), e.weaponType);
                    }

                    if (e.shotNumber == 0 && weaponMediaInfo.startSmoke)
                    {
                        createWeaponSmoke(simVectorToFloat(e.firePoint));
                    }
                },
                [&](const UnitArrivedEvent& e) {
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Arrived1);
                    }
                },
                [&](const UnitActivatedEvent& e) {
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Activate);

                        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && *selectedUnit == e.unitId)
                        {
                            onOff.next(true);
                        }
                    }
                },
                [&](const UnitDeactivatedEvent& e) {
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Deactivate);

                        if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit && *selectedUnit == e.unitId)
                        {
                            onOff.next(false);
                        }
                    }
                },
                [&](const UnitCompleteEvent& e) {
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::UnitComplete);
                    }
                },
                [&](const EmitParticleFromPieceEvent& e) {
                    if (!simulation.unitExists(e.unitId))
                    {
                        return;
                    }

                    switch (e.sfxType)
                    {
                        case EmitParticleFromPieceEvent::SfxType::LightSmoke:
                            emitLightSmokeFromPiece(e.unitId, e.pieceName);
                            break;
                        case EmitParticleFromPieceEvent::SfxType::BlackSmoke:
                            emitBlackSmokeFromPiece(e.unitId, e.pieceName);
                            break;
                        case EmitParticleFromPieceEvent::SfxType::Wake1:
                            emitWake1FromPiece(e.unitId, e.pieceName);
                            break;
                        default:
                            throw std::logic_error("unknown particle type");
                    }
                },
                [&](const UnitSpawnedEvent& e) {
                    // initialise local-player-specific UI data
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        const auto& unitDefinition = simulation.unitDefinitions.at(unit->get().unitType);
                        unitGuiInfos.insert_or_assign(e.unitId, UnitGuiInfo{unitDefinition.builder ? UnitGuiInfo::Section::Build : UnitGuiInfo::Section::Orders, 0});
                    }
                },

                [&](const UnitDiedEvent& e) {
                    const auto& unitDefinition = simulation.unitDefinitions.at(e.unitType);

                    if (!unitDefinition.explodeAs.empty())
                    {
                        switch (e.deathType)
                        {
                            case UnitDiedEvent::DeathType::NormalExploded:
                                doProjectileImpact(e.position, unitDefinition.explodeAs, ImpactType::Normal);
                                break;
                            case UnitDiedEvent::DeathType::WaterExploded:
                                doProjectileImpact(e.position, unitDefinition.explodeAs, ImpactType::Water);
                                break;
                            case UnitDiedEvent::DeathType::Deleted:
                                // do nothing
                                break;
                        }
                    }

                    deselectUnit(e.unitId);

                    if (hoveredUnit && *hoveredUnit == e.unitId)
                    {
                        hoveredUnit = std::nullopt;
                    }

                    unitGuiInfos.erase(e.unitId);
                },
                [&](const UnitStartedBuildingEvent& e) {
                    auto unit = tryGetUnit(e.unitId);
                    if (unit)
                    {
                        playUnitNotificationSound(unit->get().owner, unit->get().unitType, UnitSoundType::Build);
                    }
                },
                [&](const ProjectileSpawnedEvent& e) {
                    projectileRenderInfos.insert({e.projectileId, ProjectileRenderInfo{getGameTime()}});
                },
                [&](const ProjectileDiedEvent& e) {
                    const auto& weaponMediaInfo = gameMediaDatabase.getWeapon(e.weaponType);
                    if (weaponMediaInfo.endSmoke)
                    {
                        createLightSmoke(simVectorToFloat(e.position));
                    }

                    projectileRenderInfos.erase(e.projectileId);

                    switch (e.deathType)
                    {
                        case ProjectileDiedEvent::DeathType::NormalImpact:
                            doProjectileImpact(e.position, e.weaponType, ImpactType::Normal);
                            break;
                        case ProjectileDiedEvent::DeathType::WaterImpact:
                            doProjectileImpact(e.position, e.weaponType, ImpactType::Water);
                            break;
                        case ProjectileDiedEvent::DeathType::OutOfBounds:
                        case ProjectileDiedEvent::DeathType::EndOfLife:
                            // do nothing
                            break;
                    }
                });
        }

        simulation.events.clear();
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

    void GameScene::doProjectileImpact(const SimVector& position, const std::string& weaponType, ImpactType impactType)
    {
        playWeaponImpactSound(simVectorToFloat(position), weaponType, impactType);
        spawnWeaponImpactExplosion(simVectorToFloat(position), weaponType, impactType);
    }

    void GameScene::createLightSmoke(const Vector3f& position)
    {
        spawnSmoke(position, "FX", "smoke 1", ParticleFinishTimeEndOfFrames(), GameTime(2));
    }

    void GameScene::createWeaponSmoke(const Vector3f& position)
    {
        auto anim = sceneContext.textureService->getGafEntry("anims/FX.GAF", "smoke 1");
        spawnSmoke(position, "FX", "smoke 1", ParticleFinishTimeFixedTime{simulation.gameTime + GameTime(30)}, GameTime(15));
    }

    void GameScene::emitLightSmokeFromPiece(UnitId unitId, const std::string& pieceName)
    {
        auto position = simulation.getUnitPiecePosition(unitId, pieceName);
        spawnSmoke(simVectorToFloat(position), "FX", "smoke 1", ParticleFinishTimeEndOfFrames(), GameTime(2));
    }

    void GameScene::emitBlackSmokeFromPiece(UnitId unitId, const std::string& pieceName)
    {
        auto position = simulation.getUnitPiecePosition(unitId, pieceName);
        spawnSmoke(simVectorToFloat(position), "FX", "smoke 2", ParticleFinishTimeEndOfFrames(), GameTime(2));
    }

    float randomFloat(float low, float high)
    {
        return low + ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (high - low));
    }

    void GameScene::emitWake1FromPiece(UnitId unitId, const std::string& pieceName)
    {
        const auto& unit = getUnit(unitId);
        const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
        auto pieceTransform = toFloatMatrix(simulation.getUnitPieceTransform(unitId, pieceName));
        const auto& pieceMesh = gameMediaDatabase.getUnitPieceMesh(unitDefinition.objectName, pieceName).value().get();
        auto spawnPosition = pieceTransform * pieceMesh.firstVertexPosition;
        auto otherVertexPosition = pieceTransform * pieceMesh.secondVertexPosition;

        // Travel at around 10px per second -- so 1/3rd of a pixel per tick.
        // Last for about 4 seconds, travelling 40 pixels in total.
        auto velocity = (otherVertexPosition - spawnPosition).normalized() / 3.0f;
        auto duration = GameTime(120);

        const auto variation = 4.0f;

        auto spawnPosition1 = Vector3f(
            spawnPosition.x + randomFloat(-variation, variation),
            spawnPosition.y,
            spawnPosition.z + randomFloat(-variation, variation));

        auto spawnPosition2 = Vector3f(
            spawnPosition.x + randomFloat(-variation, variation),
            spawnPosition.y,
            spawnPosition.z + randomFloat(-variation, variation));

        spawnWake(spawnPosition1, velocity, duration);
        spawnWake(spawnPosition2, velocity, duration);
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

    struct CorpseSpawnInfo
    {
        std::string featureName;
        SimVector position;
        SimAngle rotation;
    };

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
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) { p.setToggledOn(std::holds_alternative<AttackCursorMode>(v)); }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "MOVE"))
        {
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) { p.setToggledOn(std::holds_alternative<MoveCursorMode>(v)); }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "DEFEND"))
        {
            p->get().addSubscription(cursorMode.subscribe([&p = p->get()](const auto& v) { p.setToggledOn(std::holds_alternative<GuardCursorMode>(v)); }));
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
                } }));
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*currentPanel, "ONOFF"))
        {
            p->get().addSubscription(onOff.subscribe([&p = p->get()](const auto& v) { p.setStage(v ? 1 : 0); }));
        }

        currentPanel->groupMessages().subscribe([this](const auto& msg) {
            if (auto activateMessage = std::get_if<ActivateMessage>(&msg.message); activateMessage != nullptr)
            {
                onMessage(msg.controlName, activateMessage->type);
            } });
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
        else if (matchesWithSidePrefix("RECLAIM", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (std::holds_alternative<ReclaimCursorMode>(cursorMode.getValue()))
            {
                cursorMode.next(NormalCursorMode());
            }
            else
            {
                cursorMode.next(ReclaimCursorMode());
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
                auto pages = getBuildPageCount(builderGuisDatabase, unit.unitType);
                guiInfo.currentBuildPage = (guiInfo.currentBuildPage + 1) % pages;

                auto buildPanelDefinition = getBuilderGui(builderGuisDatabase, unit.unitType, guiInfo.currentBuildPage);
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
                auto pages = getBuildPageCount(builderGuisDatabase, unit.unitType);
                assert(pages != 0);
                guiInfo.currentBuildPage = guiInfo.currentBuildPage == 0 ? pages - 1 : guiInfo.currentBuildPage - 1;

                auto buildPanelDefinition = getBuilderGui(builderGuisDatabase, unit.unitType, guiInfo.currentBuildPage);
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

                auto buildPanelDefinition = getBuilderGui(builderGuisDatabase, unit.unitType, guiInfo.currentBuildPage);
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
        else if (isValidUnitType(simulation, message))
        {
            if (sounds.addBuild)
            {
                sceneContext.audioService->playSound(*sounds.addBuild);
            }

            if (auto selectedUnit = getSingleSelectedUnit(); selectedUnit)
            {
                const auto& unit = getUnit(*selectedUnit);
                const auto& unitDefinition = simulation.unitDefinitions.at(unit.unitType);
                if (unitDefinition.isMobile)
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
            const auto& unitDefinition = simulation.unitDefinitions.at(e.second.unitType);
            if (!e.second.isSelectableBy(unitDefinition, localPlayerId))
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
        auto selectionSound = getSound(simulation, gameMediaDatabase, unit.unitType, UnitSoundType::Select1);
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
            auto selectionSound = getSound(simulation, gameMediaDatabase, unit.unitType, UnitSoundType::Select1);
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
            auto buildPanelDefinition = getBuilderGui(builderGuisDatabase, unit.unitType, guiInfo.currentBuildPage);
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
                    simulation.activateUnit(unitCommand.unit);
                }
                else
                {
                    simulation.deactivateUnit(unitCommand.unit);
                }
            });
    }

    bool GameScene::leftClickMode() const
    {
        return sceneContext.globalConfig->leftClickInterfaceMode;
    }

    void GameScene::spawnExplosion(const Vector3f& position, const AnimLocation& anim)
    {
        Particle particle;
        particle.position = position;
        particle.velocity = Vector3f(0.0f, 0.0f, 0.0f);
        particle.renderType = ParticleRenderTypeSprite{
            anim.gafName,
            anim.animName,
            ParticleFinishTimeEndOfFrames(),
            GameTime(2),
            false,
        };
        particle.startTime = simulation.gameTime;

        particles.push_back(particle);
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

    void GameScene::spawnSmoke(const Vector3f& position, const std::string& gaf, const std::string& anim, ParticleFinishTime duration, GameTime frameDuration)
    {
        Particle particle;
        particle.position = position;
        particle.velocity = Vector3f(0.0f, 0.5f, 0.0f);
        particle.renderType = ParticleRenderTypeSprite{
            gaf,
            anim,
            duration,
            frameDuration,
            true,
        };
        particle.startTime = simulation.gameTime;

        particles.push_back(particle);
    }

    void GameScene::spawnWake(const Vector3f& position, const Vector3f& velocity, GameTime duration)
    {
        Particle particle;
        particle.position = position;
        particle.velocity = velocity;
        particle.renderType = ParticleRenderTypeWake{
            simulation.gameTime + duration};
        particle.startTime = simulation.gameTime;

        particles.push_back(particle);
    }

    void GameScene::recreateWorldRenderTextures()
    {
        worldFrameBuffer = sceneContext.graphics->createFrameBuffer(worldViewport.width(), worldViewport.height());
        dodgeMask = sceneContext.graphics->createEmptyTexture(worldViewport.width(), worldViewport.height());
    }

    void GameScene::nudgeCamera(int millisecondsElapsed, const Rectangle2f& cameraConstraint, int directionX, int directionZ)
    {
        assert(directionX == 1 || directionX == 0 || directionX == -1);
        assert(directionZ == 1 || directionZ == 0 || directionZ == -1);

        // The player can only nudge the camera in free mode.
        // If the camera is in a different mode, try and transition out of it.
        cameraControlState = match(
            cameraControlState,
            [&](const CameraControlStateTrackingUnit&) -> CameraControlState {
                return CameraControlStateFree();
            },
            [&](const CameraControlStateFree& s) -> CameraControlState {
                return s;
            },
            [&](const CameraControlStateMiddleMousePan& s) -> CameraControlState {
                // Middle mouse pan takes precedence over nudging the camera.
                return s;
            });

        // Only nudge the camera if it is now in free mode.
        match(
            cameraControlState,
            [&](const CameraControlStateFree&) {
                const float speed = CameraPanSpeed * millisecondsElapsed / 1000.0f;

                auto dx = directionX * speed;
                auto dz = directionZ * speed;
                const auto& cameraPos = worldCameraState.position;
                auto newPos = cameraConstraint.clamp(Vector2f(cameraPos.x + dx, cameraPos.z + dz));

                worldCameraState.position = Vector3f(newPos.x, cameraPos.y, newPos.y);
            },
            [&](const CameraControlStateTrackingUnit&) {
                // do nothing
            },
            [&](const CameraControlStateMiddleMousePan&) {
                // do nothing
            });
    }

    void GameScene::relocateCamera(const Rectangle2f& cameraConstraint, float x, float z)
    {
        // The player can only relocate the camera in free mode.
        // If the camera is in a different mode, try and transition out of it.
        cameraControlState = match(
            cameraControlState,
            [&](const CameraControlStateTrackingUnit&) -> CameraControlState {
                return CameraControlStateFree();
            },
            [&](const CameraControlStateFree& s) -> CameraControlState {
                return s;
            },
            [&](const CameraControlStateMiddleMousePan& s) -> CameraControlState {
                // Middle mouse pan takes precedence over relocating the camera.
                return s;
            });

        auto newCameraPos = cameraConstraint.clamp(Vector2f(x, z));
        worldCameraState.position = Vector3f(newCameraPos.x, worldCameraState.position.y, newCameraPos.y);
    }

    std::optional<std::string> GameScene::getUnitBuildButtonUnderCursor() const
    {
        auto cursorPosition = getMousePosition();
        auto control = currentPanel->findAtPosition<UiStagedButton>(cursorPosition.x, cursorPosition.y);
        if (!control)
        {
            return std::nullopt;
        }

        const auto& name = control->get().getName();
        if (!isValidUnitType(simulation, name))
        {
            return std::nullopt;
        }

        return name;
    }
}
