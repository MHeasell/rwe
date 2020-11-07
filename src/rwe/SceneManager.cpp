#include "SceneManager.h"
#include <rwe/SceneContext.h>

namespace rwe
{
    std::optional<MouseButtonEvent::MouseButton> convertSdlMouseButton(Uint8 button)
    {
        switch (button)
        {
            case SDL_BUTTON_LEFT:
                return MouseButtonEvent::MouseButton::Left;
            case SDL_BUTTON_MIDDLE:
                return MouseButtonEvent::MouseButton::Middle;
            case SDL_BUTTON_RIGHT:
                return MouseButtonEvent::MouseButton::Right;
            default:
                return std::nullopt;
        }
    }
    SceneManager::SceneManager(
        SdlContext* sdl,
        SDL_Window* window,
        GraphicsContext* graphics,
        TimeService* timeService,
        ImGuiContext* imGuiContext,
        CursorService* cursorService,
        GlobalConfig* globalConfig,
        UiRenderService&& uiRenderService)
        : currentScene(),
          nextScene(),
          sdl(sdl),
          window(window),
          graphics(graphics),
          timeService(timeService),
          imGuiContext(imGuiContext),
          cursorService(cursorService),
          globalConfig(globalConfig),
          uiRenderService(std::move(uiRenderService)),
          requestedExit(false)
    {
    }

    void SceneManager::setNextScene(std::shared_ptr<Scene> scene)
    {
        nextScene = std::move(scene);
    }

    void dispatchToScene(const SDL_Event& event, SceneManager::Scene& currentScene)
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                currentScene.onKeyDown(event.key.keysym);
                break;
            case SDL_KEYUP:
                currentScene.onKeyUp(event.key.keysym);
                break;
            case SDL_MOUSEBUTTONDOWN:
            {
                auto button = convertSdlMouseButton(event.button.button);
                if (!button)
                {
                    break;
                }

                MouseButtonEvent e(event.button.x, event.button.y, *button);
                currentScene.onMouseDown(e);
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                auto button = convertSdlMouseButton(event.button.button);
                if (!button)
                {
                    break;
                }

                MouseButtonEvent e(event.button.x, event.button.y, *button);
                currentScene.onMouseUp(e);
                break;
            }
            case SDL_MOUSEMOTION:
            {
                MouseMoveEvent e(event.motion.x, event.motion.y);
                currentScene.onMouseMove(e);
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                MouseWheelEvent e(event.wheel.x, event.wheel.y);
                currentScene.onMouseWheel(e);
                break;
            }

            default:
                // skip unrecognised events
                break;
        }
    }

    void SceneManager::execute()
    {
        while (!requestedExit)
        {
            if (nextScene)
            {
                currentScene = std::move(nextScene);
                currentScene->init();
            }

            auto startTime = timeService->getTicks();
            auto timeElapsed = lastFrameStartTime == 0 ? 0 : startTime - lastFrameStartTime;

            SDL_Event event;
            while (sdl->pollEvent(&event))
            {
                if (imGuiContext->processEvent(event))
                {
                    continue;
                }

                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F12)
                {
                    showDebugWindow = true;
                    continue;
                }

                if (event.type == SDL_QUIT)
                {
                    return;
                }

                dispatchToScene(event, *currentScene);
            }

            if (imGuiContext->io->WantCaptureMouse)
            {
                imGuiContext->io->ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
            }
            else
            {
                imGuiContext->io->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
            }
            imGuiContext->newFrame(window);
            currentScene->update(timeElapsed);
            if (showDemoWindow)
            {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }
            renderDebugWindow();
            imGuiContext->render();

            graphics->clear();
            currentScene->render();

            if (!imGuiContext->io->WantCaptureMouse)
            {
                sdl->showCursor(SDL_FALSE);
                cursorService->render(uiRenderService);
            }

            imGuiContext->renderDrawData();

            sdl->glSwapWindow(window);

            auto finishTime = timeService->getTicks();
            lastFrameDurationMs = finishTime - startTime;
            lastFrameStartTime = startTime;
        }
    }

    void SceneManager::requestExit()
    {
        requestedExit = true;
    }

    void SceneManager::renderDebugWindow()
    {
        if (!showDebugWindow)
        {
            return;
        }

        ImGui::Begin("Global Debug", &showDebugWindow);
        ImGui::Text("Last frame time: %dms", lastFrameDurationMs);
        ImGui::Separator();
        ImGui::Checkbox("Left click interface mode", &globalConfig->leftClickInterfaceMode);
        ImGui::Separator();
        if (ImGui::Button("Grab Mouse"))
        {
            sdl->setWindowGrab(window, SDL_TRUE);
        }
        ImGui::SameLine();
        if (ImGui::Button("Ungrab Mouse"))
        {
            sdl->setWindowGrab(window, SDL_FALSE);
        }
        ImGui::Separator();
        if (ImGui::Button("Show Demo Window"))
        {
            showDemoWindow = true;
        }
        ImGui::End();
    }
}
