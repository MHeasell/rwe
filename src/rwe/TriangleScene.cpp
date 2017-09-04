#include "TriangleScene.h"

#include <rwe/math/Vector3f.h>
#include <rwe/OtherTriangleScene.h>

namespace rwe
{
    void TriangleScene::render(GraphicsContext& graphics)
    {
        Vector3f a(0.0f, 0.0f, 0.0f);
        Vector3f b(0.5f, 0.0f, 0.0f);
        Vector3f c(0.0f, 0.5f, 0.0f);
        graphics.drawTriangle(a, b, c);
    }

    TriangleScene::TriangleScene(SceneManager* sceneManager) : sceneManager(sceneManager)
    {
    }

    void TriangleScene::onKeyDown(const SDL_Keysym& /*key*/)
    {
        sceneManager->setNextScene(std::make_unique<OtherTriangleScene>(sceneManager));
    }
}
