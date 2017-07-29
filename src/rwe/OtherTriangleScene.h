#ifndef RWE_QUADSCENE_H
#define RWE_QUADSCENE_H

#include <rwe/SceneManager.h>

namespace rwe
{
    class OtherTriangleScene final : public SceneManager::Scene
    {
    public:
        void onKeyDown(SceneManager& sceneManager, const SDL_Keysym& key) final;
        void render(GraphicsContext& graphics) final;
    };
}

#endif
