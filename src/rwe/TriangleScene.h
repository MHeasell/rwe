#ifndef RWE_TRIANGLESCENE_H
#define RWE_TRIANGLESCENE_H

#include <rwe/SceneManager.h>
#include <rwe/GraphicsContext.h>

namespace rwe
{
    class TriangleScene final : public SceneManager::Scene
    {
    private:
        SceneManager* sceneManager;

    public:
        TriangleScene(SceneManager* sceneManager);

        void onKeyDown(const SDL_Keysym& key) override;
        void render(GraphicsContext& graphics) override;
    };
}

#endif
