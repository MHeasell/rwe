#ifndef RWE_MAINMENUSCENE_H
#define RWE_MAINMENUSCENE_H

#include <rwe/SceneManager.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/TextureService.h>
#include <rwe/camera/UiCamera.h>
#include <rwe/AudioService.h>
#include <rwe/tdf/SimpleTdfAdapter.h>

namespace rwe
{
    class UiPanelScene : public SceneManager::Scene
    {
    private:
        AudioService* audioService;
        TdfBlock* soundLookup;

        std::vector<UiPanel> panelStack;
        UiCamera camera;

        AudioService::LoopToken bgm;

    public:
        UiPanelScene(AudioService* audioService, TdfBlock* soundLookup, float width, float height);

        void init() override;

        void render(GraphicsContext& context) override;

        void onMouseDown(MouseButtonEvent event) override;

        void onMouseUp(MouseButtonEvent event) override;

        void onMouseMove(MouseMoveEvent event) override;

        void goToPreviousMenu();

        bool hasPreviousMenu() const;

        void goToMenu(UiPanel&& panel);

    private:
        AudioService::LoopToken startBgm();
    };
}

#endif
