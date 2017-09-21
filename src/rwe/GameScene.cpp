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
}
