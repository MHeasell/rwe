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
        float speed = 32.0f;
        if (keysym.sym == SDLK_UP)
        {
            camera.translate(Vector3f(0.0f, 0.0f, -speed));
        }
        else if (keysym.sym == SDLK_DOWN)
        {

            camera.translate(Vector3f(0.0f, 0.0f, speed));
        }
        else if (keysym.sym == SDLK_LEFT)
        {
            camera.translate(Vector3f(-speed, 0.0f, 0.0f));
        }
        else if (keysym.sym == SDLK_RIGHT)
        {
            camera.translate(Vector3f(speed, 0.0f, 0.0f));
        }
    }
}
