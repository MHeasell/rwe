#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/SceneManager.h>

namespace rwe
{
    class GameScene : public SceneManager::Scene
    {
    private:
        /**
         * Speed the camera pans via the arrow keys
         * in world units/second.
         */
        static constexpr float CameraPanSpeed = 1000.0f;

        CabinetCamera camera;
        MapTerrain terrain;

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

    public:
        GameScene(CabinetCamera&& camera, MapTerrain&& terrain);

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void update() override;
    };
}

#endif
