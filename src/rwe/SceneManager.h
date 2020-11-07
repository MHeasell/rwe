#pragma once

#include <memory>
#include <rwe/CursorService.h>
#include <rwe/GlobalConfig.h>
#include <rwe/ImGuiContext.h>
#include <rwe/SdlContextManager.h>
#include <rwe/events.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/rwe_time.h>
#include <stack>

namespace rwe
{
    class SceneManager
    {
    public:
        class Scene
        {
        public:
            virtual void update(int millisecondsElapsed) {}

            virtual void init() {}

            virtual void render() {}

            virtual void onKeyDown(const SDL_Keysym& /*key*/) {}

            virtual void onKeyUp(const SDL_Keysym& /*key*/) {}

            virtual void onMouseDown(MouseButtonEvent /*event*/) {}

            virtual void onMouseUp(MouseButtonEvent /*event*/) {}

            virtual void onMouseMove(MouseMoveEvent /*event*/) {}

            virtual void onMouseWheel(MouseWheelEvent /*event*/) {}

            virtual ~Scene() = default;
        };

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
        bool requestedExit;
        bool showDebugWindow{false};
        bool showDemoWindow{false};

        unsigned int lastFrameStartTime{0};
        unsigned int lastFrameDurationMs{0};

    public:
        // Number of milliseconds between each game tick.
        static const unsigned int TickInterval = 1000 / 60;

        explicit SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics, TimeService* timeService, ImGuiContext* imGuiContext, CursorService* cursorService, GlobalConfig* globalConfig, UiRenderService&& uiRenderService);
        void setNextScene(std::shared_ptr<Scene> scene);

        void execute();

        void requestExit();

    private:
        void renderDebugWindow();
    };
}
