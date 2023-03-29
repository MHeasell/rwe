#pragma once

#include <array>
#include <memory>
#include <rwe/CursorService.h>
#include <rwe/GlobalConfig.h>
#include <rwe/ImGuiContext.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/rwe_time.h>
#include <rwe/scene/Scene.h>
#include <rwe/sdl/SdlContext.h>
#include <stack>

namespace rwe
{
    class SceneManager
    {
    private:
        std::shared_ptr<Scene> currentScene;
        std::shared_ptr<Scene> nextScene;
        SdlContext* sdl;
        SDL_Window* window;
        GraphicsContext* graphics;
        TimeService* timeService;
        ImGuiContext* imGuiContext;
        CursorService* cursorService;
        GlobalConfig* globalConfig;
        UiRenderService uiRenderService;
        Viewport* const viewport;
        bool requestedExit;
        bool showDebugWindow{false};
        bool showDemoWindow{false};

        unsigned int lastFrameStartTime{0};

        std::array<float, 500> frameTimes;
        int frameTimesOffset{0};

    public:
        explicit SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics, TimeService* timeService, ImGuiContext* imGuiContext, CursorService* cursorService, GlobalConfig* globalConfig, UiRenderService&& uiRenderService, Viewport* viewport);
        void setNextScene(std::shared_ptr<Scene> scene);

        void execute();

        void requestExit();

    private:
        void renderDebugWindow();
    };
}
