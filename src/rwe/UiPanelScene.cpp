#include "UiPanelScene.h"

#include <rwe/gui.h>

namespace rwe
{
    UiPanelScene::UiPanelScene(
            AudioService* audioService,
            TdfBlock* soundLookup,
            float width,
            float height)
        : audioService(audioService),
          soundLookup(soundLookup),
          panelStack(),
          camera(width, height)
    {
    }

    void UiPanelScene::init()
    {
        bgm = startBgm();
    }

    void UiPanelScene::render(GraphicsContext& context)
    {
        context.applyCamera(camera);
        panelStack.back().render(context);
    }

    void UiPanelScene::onMouseDown(MouseButtonEvent event)
    {
        panelStack.back().mouseDown(event);
    }

    void UiPanelScene::onMouseUp(MouseButtonEvent event)
    {
        panelStack.back().mouseUp(event);
    }

    void UiPanelScene::onMouseMove(MouseMoveEvent event)
    {
        panelStack.back().mouseMove(event);
    }

    AudioService::LoopToken UiPanelScene::startBgm()
    {
        auto bgmBlock = soundLookup->findBlock("BGM");
        if (!bgmBlock)
        {
            return AudioService::LoopToken();
        }

        auto bgmName = bgmBlock->findValue("sound");
        if (!bgmName)
        {
            return AudioService::LoopToken();
        }

        auto bgm = audioService->loadSound(*bgmName);
        if (!bgm)
        {
            return AudioService::LoopToken();
        }

        return audioService->loopSound(*bgm);
    }

    void UiPanelScene::goToPreviousMenu()
    {
        panelStack.pop_back();
    }

    void UiPanelScene::goToMenu(UiPanel&& panel)
    {
        panelStack.push_back(std::move(panel));
    }

    bool UiPanelScene::hasPreviousMenu() const
    {
        return panelStack.size() > 1;
    }
}
