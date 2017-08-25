#include <GL/glew.h>
#include <rwe/SdlContextManager.h>

#include <rwe/GraphicsContext.h>
#include <rwe/SceneManager.h>

#include <memory>
#include <rwe/TriangleScene.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <rwe/tdf.h>

#include <rwe/gui.h>
#include <iostream>
#include <rwe/UiPanelScene.h>
#include <rwe/ColorPalette.h>
#include <boost/filesystem.hpp>
#include <rwe/AudioService.h>

namespace fs = boost::filesystem;

namespace rwe
{
    int run(const std::string& searchPath)
    {
        SdlContextManager sdlManager;

        auto sdlContext = sdlManager.getSdlContext();

        auto window = sdlContext->createWindow("RWE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
        assert(window != nullptr);
        auto glContext = sdlContext->glCreateContext(window.get());
        assert(glContext != nullptr);

        auto glewResult = glewInit();
        assert(glewResult == GLEW_OK);

        auto vfs = constructVfs(searchPath);

        auto paletteBytes = vfs.readFile("palettes/PALETTE.PAL");
        if (!paletteBytes)
        {
            std::cerr << "Couldn't find palette" << std::endl;
            return 1;
        }

        auto palette = readPalette(*paletteBytes);
        if (!palette)
        {
            std::cerr << "Couldn't read palette" << std::endl;
            return 1;
        }

        GraphicsContext graphics;

        TextureService textureService(&graphics, &vfs, &*palette);

        AudioService audioService(sdlContext, sdlManager.getSdlMixerContext(), &vfs);

        SceneManager sceneManager(sdlContext, window.get(), &graphics);

        // load sound definitions
        auto allSoundBytes = vfs.readFile("gamedata/ALLSOUND.TDF");
        if (!allSoundBytes)
        {
            std::cerr << "Couldn't read ALLSOUND.TDF" << std::endl;
            return 1;
        }

        std::string allSoundString(allSoundBytes->data(), allSoundBytes->size());
        auto allSoundTdf = parseTdfFromString(allSoundString);

        UiFactory uiFactory(&textureService, &audioService, &allSoundTdf);

        auto mainMenuGuiRaw = vfs.readFile("guis/MAINMENU.GUI");
        if (!mainMenuGuiRaw)
        {
            std::cerr << "Couldn't read main menu GUI" << std::endl;
            return 1;
        }

        std::string gui(mainMenuGuiRaw->data(), mainMenuGuiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            std::cerr << "Failed to parse GUI file" << std::endl;
            return 1;
        }

        auto panel = uiFactory.panelFromGuiFile("MAINMENU", *parsedGui);
        auto scene = std::make_unique<UiPanelScene>(std::move(panel));

        sceneManager.setNextScene(std::move(scene));

        // start the BGM
        auto allSoundBgm = allSoundTdf.findBlock("BGM");
        if (allSoundBgm)
        {
            auto sound = allSoundBgm->findValue("sound");
            if (sound)
            {
                audioService.loopSound(*sound);
            }
        }

        sceneManager.execute();

        return 0;
    }

}

int main(int argc, char* argv[])
{
    auto appData = std::getenv("APPDATA");
    if (appData == nullptr)
    {
        std::cerr << "Failed to detect AppData directory" << std::endl;
        return 1;
    }

    fs::path searchPath(appData);
    searchPath /= "RWE";
    searchPath /= "Data";

    return rwe::run(searchPath.string());
}
