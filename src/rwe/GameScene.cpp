#include "GameScene.h"
#include <boost/range/adaptor/map.hpp>
#include <rwe/Mesh.h>
#include <unordered_set>

namespace rwe
{
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

    GameScene::GameScene(
        SceneManager* sceneManager,
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
        : sceneManager(sceneManager),
          textureService(textureService),
          cursor(cursor),
          sdl(sdl),
          audioService(audioService),
          viewportService(viewportService),
          renderService(std::move(renderService)),
          uiRenderService(std::move(uiRenderService)),
          simulation(std::move(simulation)),
          collisionService(std::move(collisionService)),
          unitFactory(textureService, std::move(unitDatabase), std::move(meshService), &this->collisionService, palette, guiPalette),
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

        renderService.drawFlatFeatureShadows(simulation.features | boost::adaptors::map_values);
        renderService.drawFlatFeatures(simulation.features | boost::adaptors::map_values);

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

        renderService.drawUnitShadows(simulation.terrain, simulation.units | boost::adaptors::map_values);

        context.enableDepthBuffer();

        auto seaLevel = simulation.terrain.getSeaLevel();
        for (const auto& unit : (simulation.units | boost::adaptors::map_values))
        {
            renderService.drawUnit(unit, seaLevel);
        }

        renderService.drawLasers(simulation.lasers);

        context.disableDepthWrites();

        context.disableDepthTest();
        renderService.drawStandingFeatureShadows(simulation.features | boost::adaptors::map_values);
        context.enableDepthTest();

        renderService.drawStandingFeatures(simulation.features | boost::adaptors::map_values);

        context.disableDepthTest();
        renderService.drawExplosions(simulation.gameTime, simulation.explosions);
        context.enableDepthTest();

        context.enableDepthWrites();

        // UI rendering
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

                auto uiPos = uiRenderService.getCamera().getInverseViewProjectionMatrix()
                    * renderService.getCamera().getViewProjectionMatrix()
                    * unit.position;
                uiRenderService.drawHealthBar(uiPos.x, uiPos.y, static_cast<float>(unit.hitPoints) / static_cast<float>(unit.maxHitPoints));
            }
        }

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
            if (selectedUnit)
            {
                localPlayerStopUnit(*selectedUnit);
            }
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
                            localPlayerEnqueueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
                        }
                        else
                        {
                            localPlayerIssueUnitOrder(*selectedUnit, AttackOrder(*hoveredUnit));
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
                                localPlayerEnqueueUnitOrder(*selectedUnit, AttackOrder(*coord));
                            }
                            else
                            {
                                localPlayerIssueUnitOrder(*selectedUnit, AttackOrder(*coord));
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

        // Queue up commands collected from the local player
        playerCommandService.pushCommands(localPlayerId, localPlayerCommandBuffer);
        localPlayerCommandBuffer.clear();

        // Queue up commands from the computer players
        for (unsigned int i = 0; i < simulation.players.size(); ++i)
        {
            PlayerId id(i);
            if (id != localPlayerId) // FIXME: should properly check that the player is a computer
            {
                // TODO: implement computer AI logic to decide commands here
                playerCommandService.pushCommands(id, std::vector<PlayerCommand>());
            }
        }

        processActions();

        if (hasPlayerCommands())
        {
            sceneTime = nextSceneTime(sceneTime);
            simulation.gameTime = nextGameTime(simulation.gameTime);

            processPlayerCommands();

            pathFindingService.update();

            // run unit scripts
            for (auto& entry : simulation.units)
            {
                auto unitId = entry.first;
                auto& unit = entry.second;

                unitBehaviorService.update(unitId);

                unit.mesh.update(secondsElapsed);

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
                delay(SceneTimeDelta(5 * 60), [sm = sceneManager]() { sm->requestExit(); });
            }
            else if (auto drawStatus = boost::get<WinStatusDraw>(&winStatus); drawStatus != nullptr)
            {
                delay(SceneTimeDelta(5 * 60), [sm = sceneManager]() { sm->requestExit(); });
            }

            deleteDeadUnits();
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

    void GameScene::playUnitSound(UnitId /*unitId*/, const AudioService::SoundHandle& sound)
    {
        // FIXME: should play on a unit-specific channel group
        audioService->playSound(sound);
    }

    void GameScene::playSoundAt(const Vector3f& /*position*/, const AudioService::SoundHandle& sound)
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
        simulation.spawnSmoke(position, textureService->getGafEntry("anims/FX.GAF", "smoke 1"));
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

    bool GameScene::hasPlayerCommands() const
    {
        for (unsigned int i = 0; i < simulation.players.size(); ++i)
        {
            PlayerId id(i);
            if (!playerCommandService.hasCommands(id))
            {
                return false;
            }
        }

        return true;
    }

    void GameScene::processPlayerCommands()
    {
        assert(hasPlayerCommands());

        for (unsigned int i = 0; i < simulation.players.size(); ++i)
        {
            PlayerId id(i);

            auto commands = playerCommandService.getFrontCommands(id);

            PlayerCommandDispatcher dispatcher(this, id);
            for (const auto& c : commands)
            {
                boost::apply_visitor(dispatcher, c);
            }
        }

        playerCommandService.popCommands();
    }
}
