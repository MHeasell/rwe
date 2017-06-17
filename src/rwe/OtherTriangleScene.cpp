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

    void OtherTriangleScene::onKeyDown(SceneManager& sceneManager, const SDL_Keysym& key)
    {
        sceneManager.replaceScene(std::make_unique<TriangleScene>());
    }
}
