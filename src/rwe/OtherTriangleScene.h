#ifndef RWE_QUADSCENE_H
#define RWE_QUADSCENE_H

#include <rwe/SceneManager.h>

namespace rwe
{
    class OtherTriangleScene final : public SceneManager::Scene
    {
    private:
        SceneManager* sceneManager;
    public:
        OtherTriangleScene(SceneManager* sceneManager);

        void onKeyDown(const SDL_Keysym& key) override;
        void render(GraphicsContext& graphics) override;
    };
}

#endif
