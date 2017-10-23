#include "GameScene.h"
#include "Mesh.h"

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
        context.disableDepth();

        context.applyCamera(uiCamera);
        cursor->render(context);
    }

    GameScene::GameScene(
        TextureService* textureService,
        CursorService* cursor,
        MeshService&& meshService,
        CabinetCamera&& camera,
        MapTerrain&& terrain,
        SharedShaderProgramHandle&& unitTextureShader,
        SharedShaderProgramHandle&& unitColorShader)
        : textureService(textureService),
          cursor(cursor),
          meshService(std::move(meshService)),
          camera(std::move(camera)),
          terrain(std::move(terrain)),
          uiCamera(640, 480),
          unitTextureShader(std::move(unitTextureShader)),
          unitColorShader(std::move(unitColorShader))
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

    void GameScene::update()
    {
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
    }

    void GameScene::spawnUnit(const std::string& unitType, const Vector3f& position)
    {
        units.push_back(createUnit(unitType, position));
    }

    void GameScene::setCameraPosition(const Vector3f& newPosition)
    {
        camera.setPosition(newPosition);
    }

    const MapTerrain& GameScene::getTerrain() const
    {
        return terrain;
    }

    Unit GameScene::createUnit(const std::string& unitType, const Vector3f& position)
    {
        Unit unit;
        unit.position = position;
        unit.mesh = meshService.loadUnitMesh("armcom");

        return unit;
    }
}
