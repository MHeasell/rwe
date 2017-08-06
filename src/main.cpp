#include <GL/glew.h>
#include <rwe/SdlContextManager.h>

#include <rwe/GraphicsContext.h>
#include <rwe/SceneManager.h>

#include <memory>
#include <rwe/TriangleScene.h>

namespace rwe
{
    void run()
    {
        SdlContextManager sdlManager;

        auto sdlContext = sdlManager.getSdlContext();

        auto window = sdlContext->createWindow("RWE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_OPENGL);
        assert(window != nullptr);
        auto glContext = sdlContext->glCreateContext(window.get());
        assert(glContext != nullptr);

        auto glewResult = glewInit();
        assert(glewResult == GLEW_OK);

        GraphicsContext graphics;

        SceneManager sceneManager(sdlContext, window.get(), &graphics);
        sceneManager.setNextScene(std::make_unique<TriangleScene>());
        sceneManager.execute();
    }
}

int main(int argc, char* argv[])
{
    rwe::run();
    return 0;
}
