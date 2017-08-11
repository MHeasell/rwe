#include "OtherTriangleScene.h"
#include <rwe/TriangleScene.h>

namespace rwe
{

    void OtherTriangleScene::render(GraphicsContext& graphics)
    {
        Vector3f a(0.0f, 0.0f, 0.0f);
        Vector3f b(0.5f, 0.0f, 0.0f);
        Vector3f c(0.5f, 0.5f, 0.0f);
        graphics.drawTriangle(a, b, c);
    }

    void OtherTriangleScene::onKeyDown(const SDL_Keysym& /*key*/)
    {
        sceneManager->setNextScene(std::make_unique<TriangleScene>(sceneManager));
    }

    OtherTriangleScene::OtherTriangleScene(SceneManager* sceneManager) : sceneManager(sceneManager)
    {
    }
}
