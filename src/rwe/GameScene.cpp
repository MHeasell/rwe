#include "GameScene.h"
#include <boost/range/adaptor/map.hpp>
#include <rwe/Mesh.h>
#include <rwe/ui/UiStagedButton.h>
#include <spdlog/spdlog.h>
#include <unordered_set>

namespace rwe
{
#if BOOST_VERSION < 105800
    bool operator!=(const CursorMode& lhs, const CursorMode& rhs)
    {
        return !(lhs == rhs);
    }
#endif

    class LaserCollisionVisitor : public boost::static_visitor<bool>
    {
    private:
        const GameScene* scene;
        const std::optional<LaserProjectile>* laserPtr;

    public:
        LaserCollisionVisitor(const GameScene* scene, const std::optional<LaserProjectile>* laserPtr)
            : scene(scene), laserPtr(laserPtr)
        {
        }

        bool operator()(const OccupiedUnit& v) const
        {
            auto& laser = *laserPtr;
            const auto& unit = scene->getSimulation().getUnit(v.id);

            if (unit.isOwnedBy(laser->owner))
            {
                return false;
            }

            // ignore if the laser is above or below the unit
            if (laser->position.y < unit.position.y || laser->position.y > unit.position.y + unit.height)
            {
                return false;
            }

            return true;
        }
        bool operator()(const OccupiedFeature& v) const
        {
            auto& laser = *laserPtr;
            const auto& feature = scene->getSimulation().getFeature(v.id);

            // ignore if the laser is above or below the feature
            if (laser->position.y < feature.position.y || laser->position.y > feature.position.y + feature.height)
            {
                return false;
            }

            return true;
        }
        bool operator()(const OccupiedNone&) const
        {
            return false;
        }
    };

    const Rectangle2f GameScene::minimapViewport = Rectangle2f::fromTopLeft(0.0f, 0.0f, GuiSizeLeft, GuiSizeLeft);

    GameScene::GameScene(
        const SceneContext& sceneContext,
        std::unique_ptr<PlayerCommandService>&& playerCommandService,
        RenderService&& worldRenderService,
        UiRenderService&& worldUiRenderService,
        UiRenderService&& chromeUiRenderService,
        GameSimulation&& simulation,
        MovementClassCollisionService&& collisionService,
        UnitDatabase&& unitDatabase,
        MeshService&& meshService,
        std::unique_ptr<GameNetworkService>&& gameNetworkService,
        const std::shared_ptr<Sprite>& minimap,
        const std::shared_ptr<SpriteSeries>& minimapDots,
        const std::shared_ptr<Sprite>& minimapDotHighlight,
        const std::shared_ptr<Sprite>& neutralPanel,
        std::unique_ptr<UiPanel>&& ordersPanel,
        InGameSoundsInfo sounds,
        PlayerId localPlayerId)
        : sceneContext(sceneContext),
          worldViewport(ViewportService(GuiSizeLeft, GuiSizeTop, sceneContext.viewportService->width() - GuiSizeLeft - GuiSizeRight, sceneContext.viewportService->height() - GuiSizeTop - GuiSizeBottom)),
          playerCommandService(std::move(playerCommandService)),
          worldRenderService(std::move(worldRenderService)),
          worldUiRenderService(std::move(worldUiRenderService)),
          chromeUiRenderService(std::move(chromeUiRenderService)),
          simulation(std::move(simulation)),
          collisionService(std::move(collisionService)),
          unitFactory(sceneContext.textureService, std::move(unitDatabase), std::move(meshService), &this->collisionService, sceneContext.palette, sceneContext.guiPalette),
          gameNetworkService(std::move(gameNetworkService)),
          pathFindingService(&this->simulation, &this->collisionService),
          unitBehaviorService(this, &pathFindingService, &this->collisionService),
          cobExecutionService(),
          minimap(minimap),
          minimapDots(minimapDots),
          minimapDotHighlight(minimapDotHighlight),
          minimapRect(minimapViewport.scaleToFit(this->minimap->bounds)),
          neutralPanel(neutralPanel),
          ordersPanel(std::move(ordersPanel)),
          sounds(std::move(sounds)),
          localPlayerId(localPlayerId)
    {
    }

    void GameScene::init()
    {
        sceneContext.audioService->reserveChannels(reservedChannelsCount);
        gameNetworkService->start();

        attachOrdersMenuEventHandlers();
    }

    void GameScene::render(GraphicsContext& context)
    {
        auto viewportPos = worldViewport.toOtherViewport(*sceneContext.viewportService, 0, worldViewport.height());
        context.setViewport(
            viewportPos.x,
            sceneContext.viewportService->height() - viewportPos.y,
            worldViewport.width(),
            worldViewport.height());
        renderWorld(context);

        context.setViewport(0, 0, sceneContext.viewportService->width(), sceneContext.viewportService->height());
        context.disableDepthBuffer();

        renderMinimap(context);

        if (selectedUnit)
        {
            ordersPanel->render(chromeUiRenderService);
        }
        else
        {
            chromeUiRenderService.drawSpriteAbs(0.0f, 128.0f, *neutralPanel);
        }

        sceneContext.cursor->render(chromeUiRenderService);
        context.enableDepthBuffer();
    }

    void GameScene::renderMinimap(GraphicsContext& context)
    {
        // draw minimap
        chromeUiRenderService.drawSpriteAbs(minimapRect, *minimap);

        auto cameraInverse = worldRenderService.getCamera().getInverseViewProjectionMatrix();
        auto worldToMinimap = worldToMinimapMatrix(simulation.terrain, minimapRect);

        // draw minimap dots
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            auto minimapPos = worldToMinimap * unit.position;
            minimapPos.x = std::floor(minimapPos.x);
            minimapPos.y = std::floor(minimapPos.y);
            auto ownerId = unit.owner;
            auto colorIndex = getPlayer(ownerId).color;
            assert(colorIndex >= 0 && colorIndex < 10);
            chromeUiRenderService.drawSprite(minimapPos.x, minimapPos.y, *minimapDots->sprites[colorIndex]);
        }
        // highlight the minimap dot for the hovered unit
        if (hoveredUnit)
        {
            const auto& unit = getUnit(*hoveredUnit);
            auto minimapPos = worldToMinimap * unit.position;
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

    void GameScene::renderWorld(GraphicsContext& context)
    {
        context.disableDepthBuffer();

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

        if (selectedUnit && movementClassGridVisible)
        {
            const auto& unit = simulation.getUnit(*selectedUnit);
            if (unit.movementClass)
            {
                const auto& grid = collisionService.getGrid(*unit.movementClass);
                worldRenderService.drawMovementClassCollisionGrid(simulation.terrain, grid);
            }
        }

        if (selectedUnit)
        {
            worldRenderService.drawSelectionRect(getUnit(*selectedUnit));
        }

        worldRenderService.drawUnitShadows(simulation.terrain, simulation.units | boost::adaptors::map_values);

        context.enableDepthBuffer();

        auto seaLevel = simulation.terrain.getSeaLevel();
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            worldRenderService.drawUnit(unit, seaLevel);
        }

        worldRenderService.drawLasers(simulation.lasers);

        context.disableDepthWrites();

        context.disableDepthTest();
        worldRenderService.drawStandingFeatureShadows(simulation.features | boost::adaptors::map_values);
        context.enableDepthTest();

        worldRenderService.drawStandingFeatures(simulation.features | boost::adaptors::map_values);

        context.disableDepthTest();
        worldRenderService.drawExplosions(simulation.gameTime, simulation.explosions);
        context.enableDepthTest();

        context.enableDepthWrites();

        // in-world UI/overlay rendering
        context.disableDepthBuffer();

        if (healthBarsVisible)
        {
            for (const Unit& unit : (simulation.units | boost::adaptors::map_values))
            {
                if (!unit.isOwnedBy(localPlayerId))
                {
                    // only draw healthbars on units we own
                    continue;
                }

                auto uiPos = worldUiRenderService.getCamera().getInverseViewProjectionMatrix()
                    * worldRenderService.getCamera().getViewProjectionMatrix()
                    * unit.position;
                worldUiRenderService.drawHealthBar(uiPos.x, uiPos.y, static_cast<float>(unit.hitPoints) / static_cast<float>(unit.maxHitPoints));
            }
        }

        context.enableDepthBuffer();
    }

    void GameScene::onKeyDown(const SDL_Keysym& keysym)
    {
        ordersPanel->keyDown(KeyEvent(keysym.sym));

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
        ordersPanel->keyUp(KeyEvent(keysym.sym));

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
        ordersPanel->mouseDown(event);

        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            if (boost::get<AttackCursorMode>(&cursorMode.getValue()) != nullptr)
            {
                if (selectedUnit)
                {
                    if (hoveredUnit)
                    {
                        if (isShiftDown())
                        {
                            localPlayerEnqueueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
                        }
                        else
                        {
                            localPlayerIssueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
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
                                localPlayerEnqueueUnitOrder(*selectedUnit, AttackOrder(*coord));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(*selectedUnit, AttackOrder(*coord));
                                cursorMode.next(NormalCursorMode());
                            }
                        }
                    }
                }
            }
            else if (boost::get<MoveCursorMode>(&cursorMode.getValue()) != nullptr)
            {
                if (selectedUnit)
                {
                    auto coord = getMouseTerrainCoordinate();
                    if (coord)
                    {
                        if (isShiftDown())
                        {
                            localPlayerEnqueueUnitOrder(*selectedUnit, MoveOrder(*coord));
                        }
                        else
                        {
                            localPlayerIssueUnitOrder(*selectedUnit, MoveOrder(*coord));
                            cursorMode.next(NormalCursorMode());
                        }
                    }
                }
            }
            else if (auto normalCursor = boost::get<NormalCursorMode>(&cursorMode.getValue()); normalCursor != nullptr)
            {
                if (isCursorOverMinimap())
                {
                    cursorMode.next(NormalCursorMode{NormalCursorMode::State::DraggingMinimap});
                }
                else if (isCursorOverWorld())
                {
                    cursorMode.next(NormalCursorMode{NormalCursorMode::State::Selecting});
                }
            }
        }
        else if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            if (boost::get<AttackCursorMode>(&cursorMode.getValue()) != nullptr)
            {
                cursorMode.next(NormalCursorMode());
            }
            else if (boost::get<MoveCursorMode>(&cursorMode.getValue()) != nullptr)
            {
                cursorMode.next(NormalCursorMode());
            }
            else if (boost::get<NormalCursorMode>(&cursorMode.getValue()) != nullptr)
            {
                if (selectedUnit)
                {
                    if (hoveredUnit && isEnemy(*hoveredUnit))
                    {
                        if (isShiftDown())
                        {
                            localPlayerEnqueueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
                        }
                        else
                        {
                            localPlayerIssueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
                        }
                    }
                    else
                    {
                        auto coord = getMouseTerrainCoordinate();
                        if (coord)
                        {
                            if (isShiftDown())
                            {
                                localPlayerEnqueueUnitOrder(*selectedUnit, MoveOrder(*coord));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(*selectedUnit, MoveOrder(*coord));
                            }
                        }
                    }
                }
            }
        }
    }

    void GameScene::onMouseUp(MouseButtonEvent event)
    {
        ordersPanel->mouseUp(event);

        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            auto normalCursor = boost::get<NormalCursorMode>(&cursorMode.getValue());
            if (normalCursor != nullptr)
            {
                if (normalCursor->state == NormalCursorMode::State::Selecting)
                {
                    if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
                    {
                        selectedUnit = hoveredUnit;
                        fireOrders.next(getUnit(*hoveredUnit).fireOrders);
                        const auto& selectionSound = getUnit(*hoveredUnit).selectionSound;
                        if (selectionSound)
                        {
                            playSoundOnSelectChannel(*selectionSound);
                        }
                    }
                    else
                    {
                        selectedUnit = std::nullopt;
                    }

                    cursorMode.next(NormalCursorMode{NormalCursorMode::State::Up});
                }
                else if (normalCursor->state == NormalCursorMode::State::DraggingMinimap)
                {
                    cursorMode.next(NormalCursorMode{NormalCursorMode::State::Up});
                }
            }
        }
    }

    void GameScene::onMouseMove(MouseMoveEvent event)
    {
        ordersPanel->mouseMove(event);
    }

    void GameScene::onMouseWheel(MouseWheelEvent event)
    {
        ordersPanel->mouseWheel(event);
    }

    Rectangle2f computeCameraConstraint(const MapTerrain& terrain, const CabinetCamera& camera)
    {
        auto cameraHalfWidth = camera.getWidth() / 2.0f;
        auto cameraHalfHeight = camera.getHeight() / 2.0f;

        auto top = terrain.topInWorldUnits() + cameraHalfHeight;
        auto left = terrain.leftInWorldUnits() + cameraHalfWidth;
        auto bottom = terrain.bottomCutoffInWorldUnits() - cameraHalfHeight;
        auto right = terrain.rightCutoffInWorldUnits() - cameraHalfWidth;

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

    void GameScene::update()
    {
        auto& camera = worldRenderService.getCamera();
        auto cameraConstraint = computeCameraConstraint(simulation.terrain, camera);

        // update camera position from keyboard arrows
        {
            const float speed = CameraPanSpeed * SecondsPerTick;
            int directionX = (right ? 1 : 0) - (left ? 1 : 0);
            int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

            auto dx = directionX * speed;
            auto dz = directionZ * speed;
            auto& cameraPos = camera.getRawPosition();
            auto newPos = cameraConstraint.clamp(Vector2f(cameraPos.x + dx, cameraPos.z + dz));

            camera.setPosition(Vector3f(newPos.x, cameraPos.y, newPos.y));
        }

        // handle minimap dragging
        if (auto cursor = boost::get<NormalCursorMode>(&cursorMode.getValue()); cursor != nullptr)
        {
            if (cursor->state == NormalCursorMode::State::DraggingMinimap)
            {
                // ok, the cursor is dragging the minimap.
                // work out where the cursor is on the minimap,
                // convert that to the world, then set the camera's position to there
                // (clamped to map bounds)

                auto minimapToWorld = minimapToWorldMatrix(simulation.terrain, minimapRect);
                auto mousePos = getMousePosition();
                auto worldPos = minimapToWorld * Vector3f(static_cast<float>(mousePos.x) + 0.5f, static_cast<float>(mousePos.y) + 0.5, 0.0f);
                auto newCameraPos = cameraConstraint.clamp(Vector2f(worldPos.x, worldPos.z));
                camera.setPosition(Vector3f(newCameraPos.x, camera.getRawPosition().y, newCameraPos.y));
            }
        }

        hoveredUnit = getUnitUnderCursor();

        if (!isCursorOverMinimap() && !isCursorOverWorld())
        {
            // The cursor is outside the world, so over UI elements.
            sceneContext.cursor->useNormalCursor();
        }
        else if (boost::get<AttackCursorMode>(&cursorMode.getValue()) != nullptr)
        {
            sceneContext.cursor->useAttackCursor();
        }
        else if (boost::get<MoveCursorMode>(&cursorMode.getValue()) != nullptr)
        {
            sceneContext.cursor->useMoveCursor();
        }
        else if (boost::get<NormalCursorMode>(&cursorMode.getValue()) != nullptr)
        {
            if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
            {
                sceneContext.cursor->useSelectCursor();
            }
            else if (selectedUnit && getUnit(*selectedUnit).canAttack && hoveredUnit && isEnemy(*hoveredUnit))
            {
                sceneContext.cursor->useRedCursor();
            }
            else
            {
                sceneContext.cursor->useNormalCursor();
            }
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
        for (unsigned int i = 0; i < simulation.players.size(); ++i)
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

        auto averageSceneTime = gameNetworkService->estimateAvergeSceneTime(sceneTime);

        // allow skipping sim frames every so often to get back down to average.
        // We tolerate X frames of drift in either direction to cope with noisiness in the estimation.
        const unsigned int frameTolerance = 3;
        const unsigned int frameCheckInterval = 5;
        auto highSceneTime = averageSceneTime + SceneTimeDelta(frameTolerance);
        auto lowSceneTime = averageSceneTime.value <= frameTolerance ? SceneTime{0} : averageSceneTime - SceneTimeDelta(frameTolerance);
        if (sceneTime.value % frameCheckInterval != 0 || sceneTime <= highSceneTime)
        {
            tryTickGame();

            // simulate an extra frame to catch up every so often
            if (sceneTime.value % frameCheckInterval == 0 && sceneTime < lowSceneTime)
            {
                tryTickGame();
            }
        }
    }

    void GameScene::spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        auto unit = unitFactory.createUnit(unitType, owner, simulation.getPlayer(owner).color, position);

        // TODO: if we failed to add the unit throw some warning
        simulation.tryAddUnit(std::move(unit));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        worldRenderService.getCamera().setPosition(newPosition);
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
    GameScene::moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed)
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

    GameTime GameScene::getGameTime() const
    {
        return simulation.gameTime;
    }

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
            Vector3f(1.0f, 0.0f, 0.0f),
            Vector3f(0.0f, 0.0f, 1.0f),
            Vector3f(0.0f, -1.0f, 0.0f));
        auto cabinet = Matrix4f::cabinetProjection(0.0f, 0.5f);
        auto orthographic = Matrix4f::orthographicProjection(
            terrain.leftInWorldUnits(),
            terrain.rightCutoffInWorldUnits(),
            terrain.bottomCutoffInWorldUnits(),
            terrain.topInWorldUnits(),
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
            terrain.leftInWorldUnits(),
            terrain.rightCutoffInWorldUnits(),
            terrain.bottomCutoffInWorldUnits(),
            terrain.topInWorldUnits(),
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
        auto playerCommands = playerCommandService->tryPopCommands();
        if (!playerCommands)
        {
            spdlog::get("rwe")->error("Blocked waiting for player commands");
            return;
        }

        sceneTime = nextSceneTime(sceneTime);
        simulation.gameTime = nextGameTime(simulation.gameTime);

        processActions();

        processPlayerCommands(*playerCommands);

        pathFindingService.update();

        // run unit scripts
        for (auto& entry : simulation.units)
        {
            auto unitId = entry.first;
            auto& unit = entry.second;

            unitBehaviorService.update(unitId);

            unit.mesh.update(SecondsPerTick);

            cobExecutionService.run(simulation, unitId);
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
        if (auto wonStatus = boost::get<WinStatusWon>(&winStatus); wonStatus != nullptr)
        {
            delay(SceneTimeDelta(5 * 60), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
        }
        else if (auto drawStatus = boost::get<WinStatusDraw>(&winStatus); drawStatus != nullptr)
        {
            delay(SceneTimeDelta(5 * 60), [sm = sceneContext.sceneManager]() { sm->requestExit(); });
        }

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
                auto minimapPos = worldToMinimap * unit.position;
                minimapPos.x = std::floor(minimapPos.x);
                minimapPos.y = std::floor(minimapPos.y);
                auto ownerId = unit.owner;
                auto colorIndex = getPlayer(ownerId).color;
                assert(colorIndex >= 0 && colorIndex < 10);
                const auto& sprite = *minimapDots->sprites[colorIndex];
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
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            return getFirstCollidingUnit(ray);
        }

        return std::nullopt;
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
        return simulation.getFirstCollidingUnit(ray);
    }

    std::optional<Vector3f> GameScene::getMouseTerrainCoordinate() const
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
            return simulation.intersectLineWithTerrain(ray.toLine());
        }

        if (isCursorOverWorld())
        {
            auto ray = worldRenderService.getCamera().screenToWorldRay(screenToWorldClipSpace(getMousePosition()));
            return simulation.intersectLineWithTerrain(ray.toLine());
        }

        return std::nullopt;
    }

    void GameScene::localPlayerIssueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto kind = PlayerUnitCommand::IssueOrder::IssueKind::Immediate;
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::IssueOrder(order, kind)));

        const auto& unit = getUnit(unitId);
        if (unit.okSound)
        {
            playSoundOnSelectChannel(*(unit.okSound));
        }
    }

    void GameScene::localPlayerEnqueueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto kind = PlayerUnitCommand::IssueOrder::IssueKind::Queued;
        localPlayerCommandBuffer.push_back(PlayerUnitCommand(unitId, PlayerUnitCommand::IssueOrder(order, kind)));
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

    void GameScene::issueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto& unit = getUnit(unitId);
        unit.clearOrders();
        unit.addOrder(order);
    }

    void GameScene::enqueueUnitOrder(UnitId unitId, const UnitOrder& order)
    {
        auto& unit = getUnit(unitId);
        unit.addOrder(order);
    }

    void GameScene::stopUnit(UnitId unitId)
    {
        auto& unit = getUnit(unitId);
        unit.clearOrders();
    }

    void GameScene::setFireOrders(UnitId unitId, UnitFireOrders orders)
    {
        auto& unit = getUnit(unitId);
        unit.fireOrders = orders;

        if (selectedUnit && *selectedUnit == unitId)
        {
            fireOrders.next(orders);
        }
    }

    bool GameScene::isShiftDown() const
    {
        return leftShiftDown || rightShiftDown;
    }

    Unit& GameScene::getUnit(UnitId id)
    {
        return simulation.getUnit(id);
    }

    const Unit& GameScene::getUnit(UnitId id) const
    {
        return simulation.getUnit(id);
    }

    const GamePlayerInfo& GameScene::getPlayer(PlayerId player) const
    {
        return simulation.getPlayer(player);
    }

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

    GameSimulation& GameScene::getSimulation()
    {
        return simulation;
    }

    const GameSimulation& GameScene::getSimulation() const
    {
        return simulation;
    }

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
            auto seaLevel = simulation.terrain.getSeaLevel();

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
                auto cellValue = simulation.occupiedGrid.grid.tryGet(heightMapPos);
                if (cellValue)
                {
                    auto collides = boost::apply_visitor(LaserCollisionVisitor(this, &laser), cellValue->get());
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
        auto minCell = simulation.terrain.getHeightMap().clampToCoords(minPoint);
        auto maxCell = simulation.terrain.getHeightMap().clampToCoords(maxPoint);

        assert(minCell.x <= maxCell.x);
        assert(minCell.y <= maxCell.y);

        auto radiusSquared = radius * radius;

        std::unordered_set<UnitId> seenUnits;

        // for each cell
        for (std::size_t y = minCell.y; y <= maxCell.y; ++y)
        {
            for (std::size_t x = minCell.x; x <= maxCell.x; ++x)
            {
                // check if it's in range
                auto cellCenter = simulation.terrain.heightmapIndexToWorldCenter(x, y);
                Rectangle2f cellRectangle(
                    Vector2f(cellCenter.x, cellCenter.z),
                    Vector2f(MapTerrain::HeightTileWidthInWorldUnits / 2.0f, MapTerrain::HeightTileHeightInWorldUnits / 2.0f));
                auto cellDistanceSquared = cellRectangle.distanceSquared(Vector2f(position.x, position.z));
                if (cellDistanceSquared > radiusSquared)
                {
                    continue;
                }

                // check if a unit (or feature) is there
                auto occupiedType = simulation.occupiedGrid.grid.get(x, y);
                auto u = boost::get<OccupiedUnit>(&occupiedType);
                if (u == nullptr)
                {
                    continue;
                }

                // check if the unit was seen/mark as seen
                auto pair = seenUnits.insert(u->id);
                if (!pair.second) // the unit was already present
                {
                    continue;
                }

                const auto& unit = simulation.getUnit(u->id);

                // skip dead units
                if (unit.isDead())
                {
                    continue;
                }

                // add in the third dimension component to distance,
                // check if we are still in range
                auto unitDistanceSquared = createBoundingBox(unit).distanceSquared(position);
                if (unitDistanceSquared > radiusSquared)
                {
                    continue;
                }

                // apply appropriate damage
                auto damageScale = std::clamp(1.0f - (std::sqrt(unitDistanceSquared) / radius), 0.0f, 1.0f);
                auto rawDamage = laser.getDamage(unit.unitType);
                auto scaledDamage = static_cast<unsigned int>(static_cast<float>(rawDamage) * damageScale);
                applyDamage(u->id, scaledDamage);
            }
        }
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

    void GameScene::deleteDeadUnits()
    {
        for (auto it = simulation.units.begin(); it != simulation.units.end();)
        {
            const auto& unit = it->second;
            if (unit.isDead())
            {
                if (selectedUnit && *selectedUnit == it->first)
                {
                    selectedUnit = std::nullopt;
                }
                if (hoveredUnit && *hoveredUnit == it->first)
                {
                    hoveredUnit = std::nullopt;
                }

                auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
                auto footprintRegion = simulation.occupiedGrid.grid.tryToRegion(footprintRect);
                assert(!!footprintRegion);
                simulation.occupiedGrid.grid.setArea(*footprintRegion, OccupiedNone());

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
        auto min = Vector3f(footprint.x, unit.position.y, footprint.y);
        auto max = Vector3f(footprint.x + footprint.width, unit.position.y + unit.height, footprint.y + footprint.height);
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
            auto impactType = unit.position.y < simulation.terrain.getSeaLevel() ? ImpactType::Water : ImpactType::Normal;
            std::optional<LaserProjectile> projectile = simulation.createProjectileFromWeapon(unit.owner, *unit.explosionWeapon, unit.position, Vector3f(0.0f, -1.0f, 0.0f));
            doLaserImpact(projectile, impactType);
        }
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
                boost::apply_visitor(dispatcher, c);
            }
        }
    }

    void GameScene::attachOrdersMenuEventHandlers()
    {
        if (auto p = findWithSidePrefix<UiStagedButton>(*ordersPanel, "ATTACK"))
        {
            cursorMode.subscribe([&p = p->get()](const auto& v) {
                p.setToggledOn(boost::get<AttackCursorMode>(&v) != nullptr);
            });
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*ordersPanel, "MOVE"))
        {
            cursorMode.subscribe([&p = p->get()](const auto& v) {
                p.setToggledOn(boost::get<MoveCursorMode>(&v) != nullptr);
            });
        }

        if (auto p = findWithSidePrefix<UiStagedButton>(*ordersPanel, "FIREORD"))
        {
            fireOrders.subscribe([&p = p->get()](const auto& v) {
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
            });
        }

        ordersPanel->groupMessages().subscribe([this](const auto& msg) {
            if (boost::get<ActivateMessage>(&msg.message) != nullptr)
            {
                onMessage(msg.controlName);
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

    void GameScene::onMessage(const std::string& message)
    {
        if (matchesWithSidePrefix("ATTACK", message))
        {
            if (sounds.specialOrders)
            {
                sceneContext.audioService->playSound(*sounds.specialOrders);
            }

            if (boost::get<AttackCursorMode>(&cursorMode.getValue()))
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

            if (boost::get<MoveCursorMode>(&cursorMode.getValue()))
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

            if (selectedUnit)
            {
                cursorMode.next(NormalCursorMode());
                localPlayerStopUnit(*selectedUnit);
            }
        }
        else if (matchesWithSidePrefix("FIREORD", message))
        {
            if (selectedUnit)
            {
                if (sounds.setFireOrders)
                {
                    sceneContext.audioService->playSound(*sounds.setFireOrders);
                }

                auto& u = getUnit(*selectedUnit);
                auto newFireOrders = nextFireOrders(u.fireOrders);
                localPlayerSetFireOrders(*selectedUnit, newFireOrders);
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
}
