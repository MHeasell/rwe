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

        for (auto& e : dialogStack)
        {
            e.render(context);
        }
    }

    void UiPanelScene::onMouseDown(MouseButtonEvent event)
    {
        topPanel().mouseDown(event);
    }

    void UiPanelScene::onMouseUp(MouseButtonEvent event)
    {
        topPanel().mouseUp(event);
    }

    void UiPanelScene::onMouseMove(MouseMoveEvent event)
    {
        topPanel().mouseMove(event);
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
        if (!dialogStack.empty())
        {
            dialogStack.pop_back();
            return;
        }

        panelStack.pop_back();
    }

    void UiPanelScene::goToMenu(UiPanel&& panel)
    {
        panelStack.push_back(std::move(panel));
    }

    void UiPanelScene::openDialog(UiPanel&& panel)
    {
        dialogStack.push_back(std::move(panel));
    }

    bool UiPanelScene::hasPreviousMenu() const
    {
        if (!dialogStack.empty())
        {
            return true;
        }

        return panelStack.size() > 1;
    }

    UiPanel& UiPanelScene::topPanel()
    {
        if (!dialogStack.empty())
        {
            return dialogStack.back();
        }

        return panelStack.back();
    }
}
