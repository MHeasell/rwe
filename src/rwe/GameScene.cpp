#include "GameScene.h"

namespace rwe
{
    void GameScene::render(GraphicsContext& context)
    {
        context.applyCamera(camera);
        terrain.render(context, camera);
    }

    GameScene::GameScene(CabinetCamera&& camera, MapTerrain&& terrain)
        : camera(std::move(camera)), terrain(std::move(terrain))
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
}
