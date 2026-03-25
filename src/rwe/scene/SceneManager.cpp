#include "SceneManager.h"

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
        UiRenderService&& uiRenderService,
        Viewport* viewport)
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
          viewport(viewport),
          requestedExit(false)
    {
    }

    void SceneManager::setNextScene(std::shared_ptr<Scene> scene)
    {
        nextScene = std::move(scene);
    }

    void dispatchToScene(const SDL_Event& event, Scene& currentScene)
    {
        switch (event.type)
        {
            case SDL_EVENT_KEY_DOWN:
                currentScene.onKeyDown(event.key);
                break;
            case SDL_EVENT_KEY_UP:
                currentScene.onKeyUp(event.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
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
            case SDL_EVENT_MOUSE_BUTTON_UP:
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
            case SDL_EVENT_MOUSE_MOTION:
            {
                MouseMoveEvent e(event.motion.x, event.motion.y);
                currentScene.onMouseMove(e);
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL:
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

                if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F11)
                {
                    showDebugWindow = true;
                    continue;
                }

                if (event.type == SDL_EVENT_QUIT)
                {
                    return;
                }

                if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED && event.window.windowID == sdl->getWindowId(window))
                {
                    viewport->setDimensions(event.window.data1, event.window.data2);
                    continue;
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
                sdl->hideCursor();
                cursorService->render(uiRenderService);
            }

            imGuiContext->renderDrawData();

            sdl->glSwapWindow(window);

            auto finishTime = timeService->getTicks();
            auto lastFrameDurationMs = finishTime - startTime;
            lastFrameStartTime = startTime;
            frameTimes[frameTimesOffset] = lastFrameDurationMs;
            frameTimesOffset = (frameTimesOffset + 1) % 500;
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
        ImGui::Text("Last frame time: %.0fms", frameTimes[(frameTimesOffset + 499) % 500]);
        ImGui::PlotLines("Frame Times", frameTimes.data(), frameTimes.size(), frameTimesOffset);
        ImGui::Separator();
        ImGui::Checkbox("Left click interface mode", &globalConfig->leftClickInterfaceMode);
        ImGui::Separator();
        if (ImGui::Button("Grab Mouse"))
        {
            sdl->setWindowGrab(window, true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Ungrab Mouse"))
        {
            sdl->setWindowGrab(window, false);
        }
        ImGui::Separator();
        if (ImGui::Button("Show Demo Window"))
        {
            showDemoWindow = true;
        }
        ImGui::End();
    }
}
