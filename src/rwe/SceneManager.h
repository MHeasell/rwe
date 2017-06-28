#ifndef RWE_SCENEMANAGER_H
#define RWE_SCENEMANAGER_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SdlContextManager.h>

#include <stack>

namespace rwe
{
    class SceneManager
    {
    public:
        class Scene
        {
        public:
            virtual void update(SceneManager& /*sceneManager*/) {}

            virtual void render(GraphicsContext& /*graphics*/) {}

            virtual void onKeyDown(SceneManager& /*sceneManager*/, const SDL_Keysym& /*key*/) {}

            virtual ~Scene() = default;
        };

        // Number of milliseconds between each game tick.
        static const unsigned int TickInterval = 1000 / 60;

        explicit SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics);
        void pushScene(std::unique_ptr<Scene>&& scene);
        void popScene();
        void replaceScene(std::unique_ptr<Scene>&& scene);
        void replaceAllScenes(std::unique_ptr<Scene>&& scene);

        void execute();

    private:
        std::stack<std::unique_ptr<Scene>> sceneStack;
        SdlContext* sdl;
        SDL_Window* window;
        GraphicsContext* graphics;
    };
}

#endif
