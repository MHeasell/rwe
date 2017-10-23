#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/Unit.h>
#include <rwe/camera/UiCamera.h>
#include "MeshService.h"

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

        TextureService* textureService;
        CursorService* cursor;

        MeshService meshService;

        CabinetCamera camera;
        MapTerrain terrain;

        std::vector<Unit> units;

        UiCamera uiCamera;

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

        SharedShaderProgramHandle unitTextureShader;
        SharedShaderProgramHandle unitColorShader;

    public:
        GameScene(
            TextureService* textureService,
            CursorService* cursor,
            MeshService&& meshService,
            CabinetCamera&& camera,
            MapTerrain&& terrain,
            SharedShaderProgramHandle&& unitTextureShader,
            SharedShaderProgramHandle&& unitColorShader);

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void update() override;

        void spawnUnit(const std::string& unitType, const Vector3f& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

    private:
        Unit createUnit(const std::string& unitType, const Vector3f& position);
    };
}

#endif
