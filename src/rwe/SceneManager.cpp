#include "SceneManager.h"

namespace rwe
{
    SceneManager::SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics) : sdl(sdl), window(window), graphics(graphics)
    {
    }

    void SceneManager::pushScene(std::unique_ptr<Scene>&& scene)
    {
        sceneStack.push(std::move(scene));
    }

    void SceneManager::popScene()
    {
        sceneStack.pop();
    }

    void SceneManager::replaceScene(std::unique_ptr<Scene>&& scene)
    {
        sceneStack.pop();
        sceneStack.push(std::move(scene));
    }

    void SceneManager::replaceAllScenes(std::unique_ptr<Scene>&& scene)
    {
        while (!sceneStack.empty())
        {
            sceneStack.pop();
        }

        sceneStack.push(std::move(scene));
    }

    void SceneManager::execute()
    {
        if (sceneStack.empty())
        {
            return;
        }

        auto currentSimulationTime = sdl->getTicks();

        while (true)
        {
            auto currentRealTime = sdl->getTicks();

            SDL_Event event;
            while (sdl->pollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        return;
                    case SDL_KEYDOWN:
                        sceneStack.top()->onKeyDown(*this, event.key.keysym);
                        break;
                }

                if (sceneStack.empty())
                {
                    return;
                }
            }

            while (currentSimulationTime < currentRealTime)
            {
                sceneStack.top()->update(*this);
                currentSimulationTime += TickInterval;

                if (sceneStack.empty())
                {
                    return;
                }
            }

            graphics->clear();
            sceneStack.top()->render(*graphics);
            sdl->glSwapWindow(window);

            auto finishTime = sdl->getTicks();
            auto nextSimTime = currentSimulationTime + TickInterval;
            if (finishTime < nextSimTime)
            {
                sdl->delay(nextSimTime - finishTime);
            }
        }
    }
}
