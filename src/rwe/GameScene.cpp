#include "GameScene.h"
#include <rwe/Mesh.h>
#include <rwe/math/rwe_math.h>

namespace rwe
{
    GameScene::GameScene(
        TextureService* textureService,
        CursorService* cursor,
        SdlContext* sdl,
        AudioService* audioService,
        ViewportService* viewportService,
        const ColorPalette* palette,
        const ColorPalette* guiPalette,
        RenderService&& renderService,
        UiRenderService&& uiRenderService,
        GameSimulation&& simulation,
        MovementClassCollisionService&& collisionService,
        UnitDatabase&& unitDatabase,
        MeshService&& meshService,
        PlayerId localPlayerId)
        : textureService(textureService),
          cursor(cursor),
          sdl(sdl),
          audioService(audioService),
          viewportService(viewportService),
          renderService(std::move(renderService)),
          uiRenderService(std::move(uiRenderService)),
          simulation(std::move(simulation)),
          collisionService(std::move(collisionService)),
          unitFactory(std::move(unitDatabase), std::move(meshService), &this->collisionService, palette, guiPalette),
          pathFindingService(&this->simulation, &this->collisionService),
          unitBehaviorService(this, &pathFindingService, &this->collisionService),
          cobExecutionService(),
          localPlayerId(localPlayerId)
    {
    }

    void GameScene::init()
    {
        audioService->reserveChannels(reservedChannelsCount);
    }

    void GameScene::render(GraphicsContext& context)
    {
        context.disableDepthBuffer();

        renderService.drawMapTerrain(simulation.terrain);

        renderService.drawFlatFeatureShadows(simulation.features);
        renderService.drawFlatFeatures(simulation.features);

        if (occupiedGridVisible)
        {
            renderService.drawOccupiedGrid(simulation.terrain, simulation.occupiedGrid);
        }

        if (pathfindingVisualisationVisible)
        {
            renderService.drawPathfindingVisualisation(simulation.terrain, pathFindingService.lastPathDebugInfo);
        }

        if (selectedUnit && movementClassGridVisible)
        {
            const auto& unit = simulation.getUnit(*selectedUnit);
            if (unit.movementClass)
            {
                const auto& grid = collisionService.getGrid(*unit.movementClass);
                renderService.drawMovementClassCollisionGrid(simulation.terrain, grid);
            }
        }

        if (selectedUnit)
        {
            renderService.drawSelectionRect(getUnit(*selectedUnit));
        }

        renderService.drawUnitShadows(simulation.terrain, simulation.units);

        context.enableDepthBuffer();

        auto seaLevel = simulation.terrain.getSeaLevel();
        for (const auto& unit : simulation.units)
        {
            renderService.drawUnit(unit, seaLevel);
        }

        renderService.drawLasers(simulation.lasers);

        context.disableDepthWrites();

        context.disableDepthTest();
        renderService.drawStandingFeatureShadows(simulation.features);
        context.enableDepthTest();

        renderService.drawStandingFeatures(simulation.features);
        context.enableDepthWrites();

        context.disableDepthBuffer();
        cursor->render(uiRenderService);
        context.enableDepthBuffer();
    }

    void GameScene::onKeyDown(const SDL_Keysym& keysym)
    {
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
        else if (keysym.sym == SDLK_a)
        {
            if (selectedUnit)
            {
                if (boost::get<AttackCursorMode>(&cursorMode) != nullptr)
                {
                    cursorMode = NormalCursorMode();
                }
                else
                {
                    cursorMode = AttackCursorMode();
                }
            }
        }
        else if (keysym.sym == SDLK_s)
        {
            stopSelectedUnit();
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
    }

    void GameScene::onKeyUp(const SDL_Keysym& keysym)
    {
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
        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            if (boost::get<AttackCursorMode>(&cursorMode) != nullptr)
            {
                if (selectedUnit)
                {
                    if (hoveredUnit)
                    {
                        if (isShiftDown())
                        {
                            enqueueAttackOrder(*selectedUnit, *hoveredUnit);
                        }
                        else
                        {
                            issueAttackOrder(*selectedUnit, *hoveredUnit);
                            cursorMode = NormalCursorMode();
                        }
                    }
                    else
                    {
                        auto coord = getMouseTerrainCoordinate();
                        if (coord)
                        {
                            if (isShiftDown())
                            {
                                enqueueAttackGroundOrder(*selectedUnit, *coord);
                            }
                            else
                            {
                                issueAttackGroundOrder(*selectedUnit, *coord);
                                cursorMode = NormalCursorMode();
                            }
                        }
                    }
                }
            }
            else
            {
                auto normalCursor = boost::get<NormalCursorMode>(&cursorMode);
                if (normalCursor != nullptr)
                {
                    normalCursor->selecting = true;
                }
            }
        }
        else if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            if (boost::get<AttackCursorMode>(&cursorMode) != nullptr)
            {
                cursorMode = NormalCursorMode();
            }
            else if (boost::get<NormalCursorMode>(&cursorMode) != nullptr)
            {
                if (selectedUnit)
                {
                    if (hoveredUnit && isEnemy(*hoveredUnit))
                    {
                        if (isShiftDown())
                        {
                            enqueueAttackOrder(*selectedUnit, *hoveredUnit);
                        }
                        else
                        {
                            issueAttackOrder(*selectedUnit, *hoveredUnit);
                        }
                    }
                    else
                    {
                        auto coord = getMouseTerrainCoordinate();
                        if (coord)
                        {
                            if (isShiftDown())
                            {
                                enqueueMoveOrder(*selectedUnit, *coord);
                            }
                            else
                            {
                                issueMoveOrder(*selectedUnit, *coord);
                            }
                        }
                    }
                }
            }
        }
    }

    void GameScene::onMouseUp(MouseButtonEvent event)
    {
        if (event.button == MouseButtonEvent::MouseButton::Left)
        {
            auto normalCursor = boost::get<NormalCursorMode>(&cursorMode);
            if (normalCursor != nullptr)
            {
                if (normalCursor->selecting)
                {
                    if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
                    {
                        selectedUnit = hoveredUnit;
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
                }
            }
        }
    }

    void GameScene::update()
    {
        simulation.gameTime = nextGameTime(simulation.gameTime);

        float secondsElapsed = static_cast<float>(SceneManager::TickInterval) / 1000.0f;
        const float speed = CameraPanSpeed * secondsElapsed;
        int directionX = (right ? 1 : 0) - (left ? 1 : 0);
        int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

        auto& camera = renderService.getCamera();
        auto left = camera.getRawPosition().x - (camera.getWidth() / 2.0f);
        auto right = camera.getRawPosition().x + (camera.getWidth() / 2.0f);
        auto top = camera.getRawPosition().z - (camera.getHeight() / 2.0f);
        auto bottom = camera.getRawPosition().z + (camera.getHeight() / 2.0f);

        auto mindx = simulation.terrain.leftInWorldUnits() - left;
        auto maxdx = simulation.terrain.rightCutoffInWorldUnits() - right;
        auto mindz = simulation.terrain.topInWorldUnits() - top;
        auto maxdz = simulation.terrain.bottomCutoffInWorldUnits() - bottom;

        auto dx = std::clamp(directionX * speed, mindx, maxdx);
        auto dz = std::clamp(directionZ * speed, mindz, maxdz);

        camera.translate(Vector3f(dx, 0.0f, dz));

        hoveredUnit = getUnitUnderCursor();

        if (boost::get<AttackCursorMode>(&cursorMode) != nullptr)
        {
            cursor->useAttackCursor();
        }
        else if (boost::get<NormalCursorMode>(&cursorMode) != nullptr)
        {
            if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
            {
                cursor->useSelectCursor();
            }
            else if (selectedUnit && getUnit(*selectedUnit).canAttack && hoveredUnit && isEnemy(*hoveredUnit))
            {
                cursor->useRedCursor();
            }
            else
            {
                cursor->useNormalCursor();
            }
        }

        pathFindingService.update();

        // run unit scripts
        for (unsigned int i = 0; i < simulation.units.size(); ++i)
        {
            UnitId unitId(i);
            auto& unit = simulation.units[i];

            unitBehaviorService.update(unitId);

            unit.mesh.update(secondsElapsed);

            cobExecutionService.run(simulation, unitId);
        }

        updateLasers();
    }

    void GameScene::spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        auto unit = unitFactory.createUnit(unitType, owner, simulation.getPlayer(owner).color, position);

        // TODO: if we failed to add the unit throw some warning
        simulation.tryAddUnit(std::move(unit));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        renderService.getCamera().setPosition(newPosition);
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
        audioService->playSoundIfFree(handle, UnitSelectChannel);
    }

    void GameScene::playUnitSound(UnitId unitId, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a unit-specific channel group
        audioService->playSound(sound);
    }

    void GameScene::playSoundAt(const Vector3f& position, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a position-aware channel
        audioService->playSound(sound);
    }

    std::optional<UnitId> GameScene::getUnitUnderCursor() const
    {
        auto ray = renderService.getCamera().screenToWorldRay(screenToClipSpace(getMousePosition()));
        return getFirstCollidingUnit(ray);
    }

    Vector2f GameScene::screenToClipSpace(Point p) const
    {
        return viewportService->toClipSpace(p);
    }

    Point GameScene::getMousePosition() const
    {
        int x;
        int y;
        sdl->getMouseState(&x, &y);
        return Point(x, y);
    }

    std::optional<UnitId> GameScene::getFirstCollidingUnit(const Ray3f& ray) const
    {
        return simulation.getFirstCollidingUnit(ray);
    }

    std::optional<Vector3f> GameScene::getMouseTerrainCoordinate() const
    {
        auto ray = renderService.getCamera().screenToWorldRay(screenToClipSpace(getMousePosition()));
        return simulation.intersectLineWithTerrain(ray.toLine());
    }

    void GameScene::issueMoveOrder(UnitId unitId, Vector3f position)
    {
        auto& unit = getUnit(unitId);
        unit.clearOrders();
        unit.addOrder(createMoveOrder(position));
        if (unit.okSound)
        {
            playSoundOnSelectChannel(*(unit.okSound));
        }
    }

    void GameScene::enqueueMoveOrder(UnitId unitId, Vector3f position)
    {
        getUnit(unitId).addOrder(createMoveOrder(position));
    }

    void GameScene::issueAttackOrder(UnitId unitId, UnitId target)
    {
        auto& unit = getUnit(unitId);
        unit.clearOrders();
        unit.addOrder(createAttackOrder(target));
        if (unit.okSound)
        {
            playSoundOnSelectChannel(*(unit.okSound));
        }
    }

    void GameScene::enqueueAttackOrder(UnitId unitId, UnitId target)
    {
        getUnit(unitId).addOrder(createAttackOrder(target));
    }

    void GameScene::issueAttackGroundOrder(UnitId unitId, Vector3f position)
    {
        auto& unit = getUnit(unitId);
        unit.clearOrders();
        unit.addOrder(createAttackGroundOrder(position));
        if (unit.okSound)
        {
            playSoundOnSelectChannel(*(unit.okSound));
        }
    }

    void GameScene::enqueueAttackGroundOrder(UnitId unitId, Vector3f position)
    {
        getUnit(unitId).addOrder(createAttackGroundOrder(position));
    }

    void GameScene::stopSelectedUnit()
    {
        if (selectedUnit)
        {
            auto& unit = getUnit(*selectedUnit);
            unit.clearOrders();
            if (unit.okSound)
            {
                playSoundOnSelectChannel(*(unit.okSound));
            }
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
        for (auto& laser : simulation.lasers)
        {
            if (!laser)
            {
                continue;
            }

            laser->position += laser->velocity;

            // test collision with terrain
            auto terrainHeight = simulation.terrain.getHeightAt(laser->position.x, laser->position.z);
            auto seaLevel = simulation.terrain.getSeaLevel();

            // test collision with sea
            // FIXME: waterweapons should be allowed in water
            if (seaLevel > terrainHeight && laser->position.y <= seaLevel)
            {
                // destroy the projectile
                // TODO: trigger detonation/impact animation
                if (laser->soundWater)
                {
                    playSoundAt(laser->position, *laser->soundWater);
                }
                laser = std::nullopt;
            }
            else if (laser->position.y <= terrainHeight)
            {
                // destroy the projectile
                // TODO: trigger detonation/impact animation
                if (laser->soundHit)
                {
                    playSoundAt(laser->position, *laser->soundHit);
                }
                laser = std::nullopt;
            }

            // TODO: detect collision between a laser and a unit, feature, world boundary
        }
    }
}
