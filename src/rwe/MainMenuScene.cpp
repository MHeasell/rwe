#include "MainMenuScene.h"

#include <rwe/gui.h>

namespace rwe
{
    MainMenuScene::MainMenuScene(
        AudioService* audioService,
        TdfBlock* soundLookup,
        CursorService* cursor,
        float width,
        float height)
        : audioService(audioService),
          soundLookup(soundLookup),
          cursor(cursor),
          panelStack(),
          camera(width, height)
    {
    }

    void MainMenuScene::init()
    {
        bgm = startBgm();
    }

    void MainMenuScene::render(GraphicsContext& context)
    {
        context.applyCamera(camera);
        panelStack.back()->render(context);

        for (auto& e : dialogStack)
        {
            context.fillColor(0.0f, 0.0f, camera.getWidth(), camera.getHeight(), Color(0, 0, 0, 63));
            e->render(context);
        }

        cursor->render(context);
    }

    void MainMenuScene::onMouseDown(MouseButtonEvent event)
    {
        topPanel().mouseDown(event);
    }

    void MainMenuScene::onMouseUp(MouseButtonEvent event)
    {
        topPanel().mouseUp(event);
    }

    void MainMenuScene::onMouseMove(MouseMoveEvent event)
    {
        topPanel().mouseMove(event);
    }

    AudioService::LoopToken MainMenuScene::startBgm()
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

    void MainMenuScene::goToPreviousMenu()
    {
        if (!dialogStack.empty())
        {
            dialogStack.pop_back();
            return;
        }

        panelStack.pop_back();
    }

    void MainMenuScene::goToMenu(std::unique_ptr<UiPanel>&& panel)
    {
        panelStack.push_back(std::move(panel));
    }

    void MainMenuScene::openDialog(std::unique_ptr<UiPanel>&& panel)
    {
        dialogStack.push_back(std::move(panel));
    }

    bool MainMenuScene::hasPreviousMenu() const
    {
        if (!dialogStack.empty())
        {
            return true;
        }

        return panelStack.size() > 1;
    }

    UiPanel& MainMenuScene::topPanel()
    {
        if (!dialogStack.empty())
        {
            return *(dialogStack.back());
        }

        return *(panelStack.back());
    }

    void MainMenuScene::update()
    {
        topPanel().update(static_cast<float>(SceneManager::TickInterval) / 1000.0f);
    }

    void MainMenuScene::onMouseWheel(MouseWheelEvent event)
    {
        topPanel().mouseWheel(event);
    }

    void MainMenuScene::onKeyDown(const SDL_Keysym& keysym)
    {
        topPanel().keyDown(KeyEvent(keysym.sym));
    }
}
