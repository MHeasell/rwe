#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/SceneManager.h>

namespace rwe
{
    class GameScene : public SceneManager::Scene
    {
    private:
        CabinetCamera camera;
        MapTerrain terrain;

    public:
        GameScene(CabinetCamera&& camera, MapTerrain&& terrain);

        void render(GraphicsContext& context) override;
    };
}

#endif
