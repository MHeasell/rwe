#include "GameScene.h"
#include "Mesh.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    class IsCollisionVisitor : public boost::static_visitor<bool>
    {
    private:
        UnitId unitId;

    public:
        explicit IsCollisionVisitor(const UnitId& unitId) : unitId(unitId)
        {
        }

        bool operator()(const OccupiedNone&) const
        {
            return false;
        }
        bool operator()(const OccupiedUnit& u) const
        {
            return u.id != unitId;
        }
        bool operator()(const OccupiedFeature&) const
        {
            return true;
        }
    };

    GameScene::GameScene(
        TextureService* textureService,
        CursorService* cursor,
        SdlContext* sdl,
        AudioService* audioService,
        ViewportService* viewportService,
        RenderService&& renderService,
        UiRenderService&& uiRenderService,
        MeshService&& meshService,
        MapTerrain&& terrain,
        UnitDatabase&& unitDatabase,
        std::array<boost::optional<GamePlayerInfo>, 10>&& players,
        PlayerId localPlayerId)
        : textureService(textureService),
          cursor(cursor),
          sdl(sdl),
          audioService(audioService),
          viewportService(viewportService),
          renderService(std::move(renderService)),
          uiRenderService(std::move(uiRenderService)),
          meshService(std::move(meshService)),
          terrain(std::move(terrain)),
          unitDatabase(std::move(unitDatabase)),
          players(std::move(players)),
          localPlayerId(localPlayerId),
          occupiedGrid(this->terrain.getHeightMap().getWidth(), this->terrain.getHeightMap().getHeight())
    {
    }

    void GameScene::init()
    {
        audioService->reserveChannels(reservedChannelsCount);

        // add features to the occupied grid
        for (const auto& f : terrain.getFeatures())
        {
            if (!f.isBlocking())
            {
                continue;
            }

            auto footprintRegion = computeFootprintRegion(f.position, f.footprintX, f.footprintZ);
            occupiedGrid.grid.setArea(occupiedGrid.grid.clipRegion(footprintRegion), OccupiedFeature());
        }
    }

    void GameScene::render(GraphicsContext& context)
    {
        auto seaLevel = terrain.getSeaLevel();

        renderService.renderMapTerrain(terrain);

        if (occupiedGridVisible)
        {
            renderService.renderOccupiedGrid(terrain, occupiedGrid);
        }

        renderService.renderFlatFeatures(terrain);

        if (selectedUnit)
        {
            renderService.renderSelectionRect(getUnit(*selectedUnit));
        }

        // draw unit shadows
        renderService.drawUnitShadows(terrain, units);

        context.enableDepthBuffer();

        for (const auto& unit : units)
        {
            renderService.renderUnit(unit, seaLevel);
        }

        context.disableDepthWrites();
        renderService.renderStandingFeatures(terrain);
        context.enableDepthWrites();

        context.disableDepthBuffer();

        cursor->render(uiRenderService);
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
        if (event.button == MouseButtonEvent::MouseButton::Right)
        {
            if (selectedUnit)
            {
                if (!hoveredUnit)
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

    void GameScene::onMouseUp(MouseButtonEvent event)
    {
        if (event.button == MouseButtonEvent::MouseButton::Left)
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
                selectedUnit = boost::none;
            }
        }
    }

    void GameScene::update()
    {
        gameTime += 1;

        float secondsElapsed = static_cast<float>(SceneManager::TickInterval) / 1000.0f;
        const float speed = CameraPanSpeed * secondsElapsed;
        int directionX = (right ? 1 : 0) - (left ? 1 : 0);
        int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

        auto& camera = renderService.getCamera();
        auto left = camera.getRawPosition().x - (camera.getWidth() / 2.0f);
        auto right = camera.getRawPosition().x + (camera.getWidth() / 2.0f);
        auto top = camera.getRawPosition().z - (camera.getHeight() / 2.0f);
        auto bottom = camera.getRawPosition().z + (camera.getHeight() / 2.0f);

        auto mindx = terrain.leftInWorldUnits() - left;
        auto maxdx = terrain.rightCutoffInWorldUnits() - right;
        auto mindz = terrain.topInWorldUnits() - top;
        auto maxdz = terrain.bottomCutoffInWorldUnits() - bottom;

        auto dx = std::clamp(directionX * speed, mindx, maxdx);
        auto dz = std::clamp(directionZ * speed, mindz, maxdz);

        camera.translate(Vector3f(dx, 0.0f, dz));

        hoveredUnit = getUnitUnderCursor();

        if (hoveredUnit && getUnit(*hoveredUnit).isOwnedBy(localPlayerId))
        {
            cursor->useSelectCursor();
        }
        else
        {
            cursor->useNormalCursor();
        }

        // run unit scripts
        for (unsigned int i = 0; i < units.size(); ++i)
        {
            UnitId unitId(i);
            auto& unit = units[i];

            unit.update(*this, unitId, secondsElapsed);
            unit.mesh.update(secondsElapsed);
            unit.cobEnvironment->executeThreads();
        }
    }

    void GameScene::spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        UnitId unitId(units.size());
        auto unit = createUnit(unitId, unitType, owner, position);

        // set footprint area as occupied by the unit
        auto footprintRect = computeFootprintRegion(unit.position, unit.footprintX, unit.footprintZ);
        if (isCollisionAt(footprintRect, unitId))
        {
            // TODO: throw some warning that we didn't really spawn it
            return;
        }

        auto footprintRegion = occupiedGrid.grid.tryToRegion(footprintRect);
        assert(!!footprintRegion);

        occupiedGrid.grid.setArea(*footprintRegion, OccupiedUnit(unitId));

        units.push_back(std::move(unit));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        renderService.getCamera().setPosition(newPosition);
    }

    const MapTerrain& GameScene::getTerrain() const
    {
        return terrain;
    }

    void GameScene::showObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = true;
        }
    }

    void GameScene::hideObject(UnitId unitId, const std::string& name)
    {
        auto mesh = getUnit(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = false;
        }
    }

    void
    GameScene::moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed)
    {
        getUnit(unitId).moveObject(name, axis, position, speed);
    }

    void GameScene::moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position)
    {
        getUnit(unitId).moveObjectNow(name, axis, position);
    }

    void GameScene::turnObject(UnitId unitId, const std::string& name, Axis axis, float angle, float speed)
    {
        getUnit(unitId).turnObject(name, axis, angle, speed);
    }

    void GameScene::turnObjectNow(UnitId unitId, const std::string& name, Axis axis, float angle)
    {
        getUnit(unitId).turnObjectNow(name, axis, angle);
    }

    bool GameScene::isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isMoveInProgress(name, axis);
    }

    bool GameScene::isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const
    {
        return getUnit(unitId).isTurnInProgress(name, axis);
    }

    unsigned int GameScene::getGameTime() const
    {
        return gameTime;
    }

    void GameScene::playSoundOnSelectChannel(const AudioService::SoundHandle& handle)
    {
        audioService->playSoundIfFree(handle, UnitSelectChannel);
    }

    Unit GameScene::createUnit(UnitId unitId, const std::string& unitType, PlayerId owner, const Vector3f& position)
    {
        const auto& fbi = unitDatabase.getUnitInfo(unitType);
        const auto& soundClass = unitDatabase.getSoundClass(fbi.soundCategory);
        boost::optional<const MovementClass&> movementClass;
        if (!fbi.movementClass.empty())
        {
            movementClass = unitDatabase.getMovementClass(fbi.movementClass);
        }

        auto meshInfo = meshService.loadUnitMesh(fbi.objectName, getPlayer(owner)->color);

        const auto& script = unitDatabase.getUnitScript(fbi.unitName);
        auto cobEnv = std::make_unique<CobEnvironment>(this, &script, unitId);
        cobEnv->createThread("Create", std::vector<int>());
        Unit unit(meshInfo.mesh, std::move(cobEnv), std::move(meshInfo.selectionMesh));
        unit.owner = owner;
        unit.position = position;

        // These units are per-tick.
        // We divide by two here because TA ticks are 1/30 of a second,
        // where as ours are 1/60 of a second.
        unit.turnRate = (fbi.turnRate / 2.0f) * (Pif / 32768.0f); // also convert to rads
        unit.maxSpeed = fbi.maxVelocity / 2.0f;
        unit.acceleration = fbi.acceleration / 2.0f;
        unit.brakeRate = fbi.brakeRate / 2.0f;

        if (movementClass)
        {
            unit.footprintX = movementClass->footprintX;
            unit.footprintZ = movementClass->footprintZ;
        }
        else
        {
            unit.footprintX = fbi.footprintX;
            unit.footprintZ = fbi.footprintZ;
        }

        if (soundClass.select1)
        {
            unit.selectionSound = unitDatabase.getSoundHandle(*(soundClass.select1));
        }
        if (soundClass.ok1)
        {
            unit.okSound = unitDatabase.getSoundHandle(*(soundClass.ok1));
        }
        if (soundClass.arrived1)
        {
            unit.arrivedSound = unitDatabase.getSoundHandle(*(soundClass.arrived1));
        }

        return unit;
    }

    boost::optional<UnitId> GameScene::getUnitUnderCursor() const
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

    boost::optional<UnitId> GameScene::getFirstCollidingUnit(const Ray3f& ray) const
    {
        auto bestDistance = std::numeric_limits<float>::infinity();
        boost::optional<UnitId> it;

        for (unsigned int i = 0; i < units.size(); ++i)
        {
            auto distance = units[i].selectionIntersect(ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it = UnitId(i);
            }
        }

        return it;
    }

    boost::optional<Vector3f> GameScene::getMouseTerrainCoordinate() const
    {
        auto ray = renderService.getCamera().screenToWorldRay(screenToClipSpace(getMousePosition()));
        return terrain.intersectLine(ray.toLine());
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
        return units.at(id.value);
    }

    const Unit& GameScene::getUnit(UnitId id) const
    {
        return units.at(id.value);
    }

    boost::optional<GamePlayerInfo>& GameScene::getPlayer(PlayerId player)
    {
        return players.at(player.value);
    }

    const boost::optional<GamePlayerInfo>& GameScene::getPlayer(PlayerId player) const
    {
        return players.at(player.value);
    }

    DiscreteRect
    GameScene::computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const
    {
        auto halfFootprintX = static_cast<float>(footprintX) * MapTerrain::HeightTileWidthInWorldUnits / 2.0f;
        auto halfFootprintZ = static_cast<float>(footprintZ) * MapTerrain::HeightTileHeightInWorldUnits / 2.0f;
        Vector3f topLeft(
            position.x - halfFootprintX,
            position.y,
            position.z - halfFootprintZ);

        auto cell = terrain.worldToHeightmapCoordinateNearest(topLeft);

        return DiscreteRect(cell.x, cell.y, footprintX, footprintZ);
    }

    bool GameScene::isCollisionAt(const DiscreteRect& rect, UnitId self) const
    {
        auto region = occupiedGrid.grid.tryToRegion(rect);
        if (!region)
        {
            return true;
        }

        for (unsigned int dy = 0; dy < region->height; ++dy)
        {
            for (unsigned int dx = 0; dx < region->width; ++dx)
            {
                const auto& cell = occupiedGrid.grid.get(region->x + dx, region->y + dy);
                if (boost::apply_visitor(IsCollisionVisitor(self), cell))
                {
                    return true;
                }
            }
        }
        return false;
    }

    void GameScene::moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId)
    {
        auto oldRegion = occupiedGrid.grid.tryToRegion(oldRect);
        assert(!!oldRegion);
        auto newRegion = occupiedGrid.grid.tryToRegion(newRect);
        assert(!!newRegion);

        occupiedGrid.grid.setArea(*oldRegion, OccupiedNone());
        occupiedGrid.grid.setArea(*newRegion, OccupiedUnit(unitId));
    }
}
