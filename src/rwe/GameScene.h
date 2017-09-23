#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/camera/UiCamera.h>

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

        CursorService* cursor;

        CabinetCamera camera;
        MapTerrain terrain;

        UiCamera uiCamera;

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

    public:
        GameScene(CursorService* cursor, CabinetCamera&& camera, MapTerrain&& terrain);

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void update() override;
    };
}

#endif
