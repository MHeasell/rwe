#ifndef RWE_MAINMENUSCENE_H
#define RWE_MAINMENUSCENE_H

#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/SceneManager.h>
#include <rwe/TextureService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/tdf/SimpleTdfAdapter.h>
#include <rwe/ui/UiPanel.h>

namespace rwe
{
    class UiPanelScene : public SceneManager::Scene
    {
    private:
        AudioService* audioService;
        TdfBlock* soundLookup;
        CursorService* cursor;

        std::vector<UiPanel> panelStack;
        std::vector<UiPanel> dialogStack;
        UiCamera camera;


        AudioService::LoopToken bgm;

    public:
        UiPanelScene(AudioService* audioService, TdfBlock* soundLookup, CursorService* cursor, float width, float height);

        void init() override;

        void render(GraphicsContext& context) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void onMouseWheel(MouseWheelEvent event) override;

        void onUiMessage(const GroupMessage& message);

        void update() override;

        void goToPreviousMenu();

        bool hasPreviousMenu() const;

        void goToMenu(UiPanel&& panel);

        void openDialog(UiPanel&& panel);

    private:
        AudioService::LoopToken startBgm();

        UiPanel& topPanel();
    };
}

#endif
