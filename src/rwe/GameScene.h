#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/Unit.h>
#include <rwe/camera/UiCamera.h>
#include "MeshService.h"
#include "UnitDatabase.h"

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
        SdlContext* sdl;

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

        SharedShaderProgramHandle selectBoxShader;

        UnitDatabase unitDatabase;

        unsigned int gameTime{0};

        boost::optional<unsigned int> hoveredUnit;
        boost::optional<unsigned int> selectedUnit;

    public:
        GameScene(
            TextureService* textureService,
            CursorService* cursor,
            SdlContext* sdl,
            MeshService&& meshService,
            CabinetCamera&& camera,
            MapTerrain&& terrain,
            SharedShaderProgramHandle&& unitTextureShader,
            SharedShaderProgramHandle&& unitColorShader,
            SharedShaderProgramHandle&& selectBoxShader,
            UnitDatabase&& unitDatabase);

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseUp(MouseButtonEvent event) override;

        void update() override;

        void spawnUnit(const std::string& unitType, const Vector3f& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

        void showObject(unsigned int unitId, const std::string& name);

        void hideObject(unsigned int unitId, const std::string& name);

        void moveObject(unsigned int unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(unsigned int unitId, const std::string& name, Axis axis, float position);

        void turnObject(unsigned int unitId, const std::string& name, Axis axis, float angle, float speed);

        void turnObjectNow(unsigned int unitId, const std::string& name, Axis axis, float angle);

        bool isPieceMoving(unsigned int unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(unsigned int unitId, const std::string& name, Axis axis) const;

        unsigned int getGameTime() const;

    private:
        Unit createUnit(unsigned int unitId, const std::string& unitType, const Vector3f& position);

        boost::optional<unsigned int> getUnitUnderCursor() const;

        Vector2f screenToClipSpace(Point p) const;

        Point getMousePosition() const;

        boost::optional<unsigned int> getFirstCollidingUnit(const Ray3f& ray) const;
    };
}

#endif
