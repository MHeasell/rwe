#include "SceneManager.h"

namespace rwe
{
    SceneManager::SceneManager(SdlContext* sdl, SDL_Window* window, GraphicsContext* graphics) :
        currentScene(), nextScene(), sdl(sdl), window(window), graphics(graphics), requestedExit(false)
    {
    }

    void SceneManager::setNextScene(std::unique_ptr<Scene>&& scene)
    {
        nextScene = std::move(scene);
    }

    void SceneManager::execute()
    {
        auto currentSimulationTime = sdl->getTicks();

        while (!requestedExit)
        {
            auto currentRealTime = sdl->getTicks();

            if (nextScene)
            {
                currentScene = std::move(nextScene);
            }

            SDL_Event event;
            while (sdl->pollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        return;
                    case SDL_KEYDOWN:
                        currentScene->onKeyDown(event.key.keysym);
                        break;
                }
            }

            while (currentSimulationTime < currentRealTime)
            {
                currentScene->update();
                currentSimulationTime += TickInterval;
            }

            graphics->clear();
            currentScene->render(*graphics);
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
