#ifndef RWE_SCENEMANAGER_H
#define RWE_SCENEMANAGER_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>
#include <rwe/events.h>

#include <stack>

namespace rwe
{
    class SceneManager
    {
    public:
        class Scene
        {
        public:
            virtual void update() {}

            virtual void render(GraphicsContext& /*graphics*/) {}

            virtual void onKeyDown(const SDL_Keysym& /*key*/) {}

            virtual void onMouseDown(MouseButtonEvent /*event*/) {}

            virtual void onMouseUp(MouseButtonEvent /*event*/) {}

            virtual void onMouseMouse(int /*x*/, int /*y*/) {}

            virtual ~Scene() = default;
        };

    private:
        std::unique_ptr<Scene> currentScene;
        std::unique_ptr<Scene> nextScene;
        SdlContext* sdl;
        SDL_Window* window;
        GraphicsContext* graphics;
        bool requestedExit;

    public:
        // Number of milliseconds between each game tick.
        static const unsigned int TickInterval = 1000 / 60;

        explicit SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics);
        void setNextScene(std::unique_ptr<Scene>&& scene);

        void execute();

    };
}

#endif
