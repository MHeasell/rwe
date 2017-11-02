#include "GameScene.h"
#include "Mesh.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    void GameScene::render(GraphicsContext& context)
    {
        context.applyCamera(camera);
        terrain.render(context, camera);
        terrain.renderFeatures(context, camera);

        context.enableDepth();
        auto viewMatrix = camera.getViewMatrix();
        auto projectionMatrix = camera.getProjectionMatrix();
        for (const auto& unit : units)
        {
            unit.render(context, unitTextureShader.get(), unitColorShader.get(), viewMatrix, projectionMatrix);
        }

        if (selectedUnit)
        {
            units[*selectedUnit].renderSelectionRect(context, viewMatrix, projectionMatrix);
        }

        context.disableDepth();

        context.applyCamera(uiCamera);
        cursor->render(context);
    }

    GameScene::GameScene(
        TextureService* textureService,
        CursorService* cursor,
        SdlContext* sdl,
        MeshService&& meshService,
        CabinetCamera&& camera,
        MapTerrain&& terrain,
        SharedShaderProgramHandle&& unitTextureShader,
        SharedShaderProgramHandle&& unitColorShader,
        UnitDatabase&& unitDatabase)
        : textureService(textureService),
          cursor(cursor),
          sdl(sdl),
          meshService(std::move(meshService)),
          camera(std::move(camera)),
          terrain(std::move(terrain)),
          uiCamera(640, 480),
          unitTextureShader(std::move(unitTextureShader)),
          unitColorShader(std::move(unitColorShader)),
          unitDatabase(std::move(unitDatabase))
    {
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
    }

    void GameScene::onMouseUp(MouseButtonEvent event)
    {
        selectedUnit = hoveredUnit;
    }

    void GameScene::update()
    {
        gameTime += 1;

        float secondsElapsed = static_cast<float>(SceneManager::TickInterval) / 1000.0f;
        const float speed = CameraPanSpeed * secondsElapsed;
        int directionX = (right ? 1 : 0) - (left ? 1 : 0);
        int directionZ = (down ? 1 : 0) - (up ? 1 : 0);

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

        if (hoveredUnit)
        {
            cursor->useSelectCursor();
        }
        else
        {
            cursor->useNormalCursor();
        }

        // run unit scripts
        for (auto& unit : units)
        {
            unit.mesh.update(secondsElapsed);
            unit.cobEnvironment->executeThreads();
        }
    }

    void GameScene::spawnUnit(const std::string& unitType, const Vector3f& position)
    {
        unsigned int unitId = units.size();
        units.push_back(createUnit(unitId, unitType, position));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        camera.setPosition(newPosition);
    }

    const MapTerrain& GameScene::getTerrain() const
    {
        return terrain;
    }

    Unit GameScene::createUnit(unsigned int unitId, const std::string& unitType, const Vector3f& position)
    {
        const auto& fbi = unitDatabase.getUnitInfo(unitType);

        auto meshInfo = meshService.loadUnitMesh(fbi.objectName);

        const auto& script = unitDatabase.getUnitScript(fbi.unitName);
        auto cobEnv = std::make_unique<CobEnvironment>(this, &script, unitId);
        cobEnv->createThread("Create", std::vector<int>());
        Unit unit(meshInfo.mesh, std::move(cobEnv), meshInfo.selectionMesh);
        unit.position = position;

        return unit;
    }

    void GameScene::showObject(unsigned int unitId, const std::string& name)
    {
        auto mesh = units.at(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = true;
        }
    }

    void GameScene::hideObject(unsigned int unitId, const std::string& name)
    {
        auto mesh = units.at(unitId).mesh.find(name);
        if (mesh)
        {
            mesh->visible = false;
        }
    }

    void
    GameScene::moveObject(unsigned int unitId, const std::string& name, Axis axis, float position, float speed)
    {
        units.at(unitId).moveObject(name, axis, position, speed);
    }

    void GameScene::moveObjectNow(unsigned int unitId, const std::string& name, Axis axis, float position)
    {
        units.at(unitId).moveObjectNow(name, axis, position);
    }

    void GameScene::turnObject(unsigned int unitId, const std::string& name, Axis axis, float angle, float speed)
    {
        units.at(unitId).turnObject(name, axis, angle, speed);
    }

    void GameScene::turnObjectNow(unsigned int unitId, const std::string& name, Axis axis, float angle)
    {
        units.at(unitId).turnObjectNow(name, axis, angle);
    }

    bool GameScene::isPieceMoving(unsigned int unitId, const std::string& name, Axis axis) const
    {
        return units.at(unitId).isMoveInProgress(name, axis);
    }

    unsigned int GameScene::getGameTime() const
    {
        return gameTime;
    }

    bool GameScene::isPieceTurning(unsigned int unitId, const std::string& name, Axis axis) const
    {
        return units.at(unitId).isTurnInProgress(name, axis);
    }

    boost::optional<unsigned int> GameScene::getUnitUnderCursor() const
    {
        auto ray = camera.screenToWorldRay(screenToClipSpace(getMousePosition()));
        return getFirstCollidingUnit(ray);
    }

    Vector2f GameScene::screenToClipSpace(Point p) const
    {
        // TODO: replace hard-coded screen size
        return convertScreenToClipSpace(640, 480, p);
    }

    boost::optional<unsigned int> GameScene::getFirstCollidingUnit(const Ray3f& ray) const
    {
        auto bestDistance = std::numeric_limits<float>::infinity();
        boost::optional<unsigned int> it;

        for (unsigned int i = 0; i < units.size(); ++i)
        {
            auto distance = units[i].selectionIntersect(ray);
            if (distance && distance < bestDistance)
            {
                bestDistance = *distance;
                it = i;
            }
        }

        return it;
    }

    Point GameScene::getMousePosition() const
    {
        int x;
        int y;
        sdl->getMouseState(&x, &y);
        return Point(x, y);
    }
}
