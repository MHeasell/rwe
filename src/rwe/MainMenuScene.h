#ifndef RWE_MAINMENUSCENE_H
#define RWE_MAINMENUSCENE_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/MapFeatureService.h>
#include <rwe/RenderService.h>
#include <rwe/SceneContext.h>
#include <rwe/SceneManager.h>
#include <rwe/SideData.h>
#include <rwe/TextureService.h>
#include <rwe/ViewportService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/tdf/TdfBlock.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/ui/UiPanel.h>

namespace rwe
{
    class MainMenuScene : public SceneManager::Scene
    {
    private:
        SceneContext sceneContext;
        TdfBlock* soundLookup;

        MapFeatureService* featureService;

        UiRenderService scaledUiRenderService;
        UiRenderService nativeUiRenderService;

        MainMenuModel model;
        UiFactory uiFactory;

        std::vector<std::unique_ptr<UiPanel>> panelStack;
        std::vector<std::unique_ptr<UiPanel>> dialogStack;

        AudioService::LoopToken bgm;

    public:
        MainMenuScene(
            const SceneContext& sceneContext,
            TdfBlock* audioLookup,
            MapFeatureService* featureService,
            float width,
            float height);

        MainMenuScene(const MainMenuScene&) = delete;
        MainMenuScene& operator=(const MainMenuScene&) = delete;
        MainMenuScene(MainMenuScene&&) = delete;
        MainMenuScene& operator=(MainMenuScene&&) = delete;

        void init() override;

        void render(GraphicsContext& context) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void onMouseWheel(MouseWheelEvent event) override;

        void onKeyDown(const SDL_Keysym& keysym) override;

        void update() override;

        void goToPreviousMenu();

        void goToMenu(std::unique_ptr<UiPanel>&& panel);

        void openDialog(std::unique_ptr<UiPanel>&& panel);

        void goToMainMenu();

        void goToSingleMenu();

        void goToSkirmishMenu();

        void openMapSelectionDialog();

        void exit();

        void message(const std::string& topic, const std::string& message, const ActivateMessage& details);

        void setCandidateSelectedMap(const std::string& mapName);

        void clearCandidateSelectedMap();

        void commitSelectedMap();

        void resetCandidateSelectedMap();

        void togglePlayer(int playerIndex);

        void incrementPlayerMetal(int playerIndex);

        void decrementPlayerMetal(int playerIndex);

        void incrementPlayerEnergy(int playerIndex);

        void decrementPlayerEnergy(int playerIndex);

        void togglePlayerSide(int playerIndex);

        void cyclePlayerColor(int playerIndex);

        void reverseCyclePlayerColor(int playerIndex);

        void cyclePlayerTeam(int playerIndex);

        void startGame();

    private:
        AudioService::LoopToken startBgm();

        UiPanel& topPanel();

        Point toScaledCoordinates(int x, int y) const;

        std::vector<std::string> getMapNames();

        void attachPlayerSelectionComponents(const std::string& guiName, UiPanel& panel);

        void attachDetailedPlayerSelectionComponents(const std::string& guiName, UiPanel& panel, int i);

        bool hasMultiplayerSchema(const std::string& mapName);
    };
}

#endif
