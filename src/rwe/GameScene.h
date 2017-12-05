#ifndef RWE_GAMESCENE_H
#define RWE_GAMESCENE_H

#include "AudioService.h"
#include "DiscreteRect.h"
#include "MeshService.h"
#include "OccupiedGrid.h"
#include "RenderService.h"
#include "UiRenderService.h"
#include "UnitDatabase.h"
#include "UnitId.h"
#include "GameSimulation.h"
#include <rwe/CursorService.h>
#include <rwe/PlayerId.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/Unit.h>
#include <rwe/ViewportService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/UnitFactory.h>

namespace rwe
{
    class GameScene : public SceneManager::Scene
    {
    private:
        static const unsigned int UnitSelectChannel = 0;

        static const unsigned int reservedChannelsCount = 1;

        /**
         * Speed the camera pans via the arrow keys
         * in world units/second.
         */
        static constexpr float CameraPanSpeed = 1000.0f;

        TextureService* textureService;
        CursorService* cursor;
        SdlContext* sdl;
        AudioService* audioService;
        ViewportService* viewportService;

        RenderService renderService;
        UiRenderService uiRenderService;

        UnitFactory unitFactory;

        GameSimulation simulation;

        PlayerId localPlayerId;

        bool left{false};
        bool right{false};
        bool up{false};
        bool down{false};

        bool leftShiftDown{false};
        bool rightShiftDown{false};

        boost::optional<UnitId> hoveredUnit;
        boost::optional<UnitId> selectedUnit;

        bool occupiedGridVisible{false};

    public:
        GameScene(
            TextureService* textureService,
            CursorService* cursor,
            SdlContext* sdl,
            AudioService* audioService,
            ViewportService* viewportService,
            RenderService&& renderService,
            UiRenderService&& uiRenderService,
            UnitFactory&& unitFactory,
            GameSimulation&& simulation,
            PlayerId localPlayerId);

        void init() override;

        void render(GraphicsContext& context) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void onKeyUp(const SDL_Keysym& keysym) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void update() override;

        void spawnUnit(const std::string& unitType, PlayerId owner, const Vector3f& position);

        void setCameraPosition(const Vector3f& newPosition);

        const MapTerrain& getTerrain() const;

        void showObject(UnitId unitId, const std::string& name);

        void hideObject(UnitId unitId, const std::string& name);

        void moveObject(UnitId unitId, const std::string& name, Axis axis, float position, float speed);

        void moveObjectNow(UnitId unitId, const std::string& name, Axis axis, float position);

        void turnObject(UnitId unitId, const std::string& name, Axis axis, float angle, float speed);

        void turnObjectNow(UnitId unitId, const std::string& name, Axis axis, float angle);

        bool isPieceMoving(UnitId unitId, const std::string& name, Axis axis) const;

        bool isPieceTurning(UnitId unitId, const std::string& name, Axis axis) const;

        unsigned int getGameTime() const;

        bool isCollisionAt(const DiscreteRect& rect, UnitId self) const;

        void playSoundOnSelectChannel(const AudioService::SoundHandle& sound);

        DiscreteRect computeFootprintRegion(const Vector3f& position, unsigned int footprintX, unsigned int footprintZ) const;

        void moveUnitOccupiedArea(const DiscreteRect& oldRect, const DiscreteRect& newRect, UnitId unitId);

        GameSimulation& getSimulation();

    private:
        boost::optional<UnitId> getUnitUnderCursor() const;

        Vector2f screenToClipSpace(Point p) const;

        Point getMousePosition() const;

        boost::optional<UnitId> getFirstCollidingUnit(const Ray3f& ray) const;

        boost::optional<Vector3f> getMouseTerrainCoordinate() const;

        void issueMoveOrder(UnitId unitId, Vector3f position);

        void enqueueMoveOrder(UnitId unitId, Vector3f position);

        void stopSelectedUnit();

        bool isShiftDown() const;

        Unit& getUnit(UnitId id);

        const Unit& getUnit(UnitId id) const;

        const GamePlayerInfo& getPlayer(PlayerId player) const;
    };
}

#endif
