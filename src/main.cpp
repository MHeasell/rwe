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
#include <rwe/MainMenuScene.h>
#include <rwe/ColorPalette.h>
#include <boost/filesystem.hpp>
#include <rwe/AudioService.h>
#include <rwe/LoadingScene.h>

#include <rwe/util.h>

namespace fs = boost::filesystem;

namespace rwe
{
    int run(const std::string& searchPath, const boost::optional<std::string>& mapName)
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
        graphics.enableCulling();

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

        CursorService cursor(sdlContext, textureService.getGafEntry("anims/CURSORS.GAF", "cursornormal"));

        sdlContext->showCursor(SDL_DISABLE);

        auto sideDataBytes = vfs.readFile("gamedata/SIDEDATA.TDF");
        if (!sideDataBytes)
        {
            throw std::runtime_error("Missing side data");
        }
        std::string sideDataString(sideDataBytes->data(), sideDataBytes->size());
        std::unordered_map<std::string, SideData> sideDataMap;
        {
            auto sideData = parseSidesFromSideData(parseTdfFromString(sideDataString));
            for (auto& side : sideData)
            {
                std::string name = side.name;
                sideDataMap.insert({std::move(name), std::move(side)});
            }
        }

        MapFeatureService featureService(&vfs);

        if (mapName)
        {
            GameParameters params{*mapName, 0};
            params.players[0] = PlayerInfo{"ARM"};
            params.players[1] = PlayerInfo{"CORE"};
            auto scene = std::make_unique<LoadingScene>(
                &vfs,
                &textureService,
                &cursor,
                &graphics,
                &featureService,
                &*palette,
                &sceneManager,
                sdlContext,
                &sideDataMap,
                AudioService::LoopToken(),
                params);
            sceneManager.setNextScene(std::move(scene));
        }
        else
        {
            auto scene = std::make_unique<MainMenuScene>(
                &sceneManager,
                &vfs,
                &textureService,
                &audioService,
                &allSoundTdf,
                &graphics,
                &featureService,
                &*palette,
                &cursor,
                sdlContext,
                &sideDataMap,
                640,
                480);
            sceneManager.setNextScene(std::move(scene));
        }

        sceneManager.execute();

        return 0;
    }
}

int main(int argc, char* argv[])
{
    try
    {
        auto searchPath = rwe::getSearchPath();
        if (!searchPath)
        {
            std::cerr << "Failed to determine data search path" << std::endl;
            return 1;
        }

        boost::optional<std::string> mapName;
        if (argc == 2)
        {
            mapName = argv[1];
        }

        return rwe::run(searchPath->string(), mapName);
    }
    catch (const std::runtime_error& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
    catch (const std::logic_error& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
}
