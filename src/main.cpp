#include <GL/glew.h>
#include <rwe/SdlContextManager.h>

#include <rwe/GraphicsContext.h>
#include <rwe/SceneManager.h>

#include <memory>
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
#include <rwe/ViewportService.h>
#include <spdlog/spdlog.h>
#include <rwe/config.h>

namespace fs = boost::filesystem;

namespace rwe
{
    int run(spdlog::logger& logger, const fs::path& localDataPath, const boost::optional<std::string>& mapName)
    {
        logger.info(ProjectNameVersion);

        ViewportService viewportService(640, 480);

        logger.info("Initializing SDL");
        SdlContextManager sdlManager;

        auto sdlContext = sdlManager.getSdlContext();

        logger.info("Initializing OpenGL context");

        int requiredOpenGlMajorVersion = 3;
        int requiredOpenGlMinorVersion = 2;

        logger.info("Requesting OpenGL version {0}.{1}, {2} profile", requiredOpenGlMajorVersion, requiredOpenGlMinorVersion, "compatibility");
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, requiredOpenGlMajorVersion) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, requiredOpenGlMinorVersion) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }

        // require a stencil buffer of some kind
        if (sdlContext->glSetAttribute(SDL_GL_STENCIL_SIZE, 1) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto window = sdlContext->createWindow("RWE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, viewportService.width(), viewportService.height(), SDL_WINDOW_OPENGL);
        if (window == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto glContext = sdlContext->glCreateContext(window.get());
        if (glContext == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto glewResult = glewInit();
        if (glewResult != GLEW_OK)
        {
            throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(glewResult)));
        }

        // log opengl context info
        logger.info("OpenGL version: {0}", glGetString(GL_VERSION));
        logger.info("OpenGL vendor: {0}", glGetString(GL_VENDOR));
        logger.info("OpenGL renderer: {0}", glGetString(GL_RENDERER));
        logger.info("OpenGL shading language version: {0}", glGetString(GL_SHADING_LANGUAGE_VERSION));
        logger.debug("OpenGL extensions:");
        int openGlExtensionCount;
        glGetIntegerv(GL_NUM_EXTENSIONS, &openGlExtensionCount);
        for (int i = 0; i < openGlExtensionCount; ++i)
        {
            logger.debug("  {0}", glGetStringi(GL_EXTENSIONS, i));
        }

        int obtainedOpenGlMajorVersion;
        int obtainedOpenGlMinorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &obtainedOpenGlMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &obtainedOpenGlMinorVersion);
        if (obtainedOpenGlMajorVersion < requiredOpenGlMajorVersion
            || (obtainedOpenGlMajorVersion == requiredOpenGlMajorVersion && obtainedOpenGlMinorVersion < requiredOpenGlMinorVersion))
        {
            throw std::runtime_error("OpenGL version did not meet requirements");
        }

        logger.info("Initializing virtual file system");
        fs::path searchPath(localDataPath);
        searchPath /= "Data";
        auto vfs = constructVfs(searchPath.string());

        logger.info("Loading palette");
        auto paletteBytes = vfs.readFile("palettes/PALETTE.PAL");
        if (!paletteBytes)
        {
            throw std::runtime_error("Couldn't find palette");
        }

        auto palette = readPalette(*paletteBytes);
        if (!palette)
        {
            throw std::runtime_error("Couldn't read palette");
        }

        logger.info("Initializing services");
        GraphicsContext graphics;
        graphics.enableCulling();

        TextureService textureService(&graphics, &vfs, &*palette);

        AudioService audioService(sdlContext, sdlManager.getSdlMixerContext(), &vfs);

        SceneManager sceneManager(sdlContext, window.get(), &graphics);

        // load sound definitions
        logger.info("Loading global sound definitions");
        auto allSoundBytes = vfs.readFile("gamedata/ALLSOUND.TDF");
        if (!allSoundBytes)
        {
            throw std::runtime_error("Couldn't read ALLSOUND.TDF");
        }

        std::string allSoundString(allSoundBytes->data(), allSoundBytes->size());
        auto allSoundTdf = parseTdfFromString(allSoundString);

        logger.info("Loading cursors");
        CursorService cursor(
            sdlContext,
            textureService.getGafEntry("anims/CURSORS.GAF", "cursornormal"),
            textureService.getGafEntry("anims/CURSORS.GAF", "cursorselect"));

        sdlContext->showCursor(SDL_DISABLE);

        logger.info("Loading side data");
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
            logger.info("Launching into map: {0}", *mapName);
            GameParameters params{*mapName, 0};
            params.players[0] = PlayerInfo{PlayerInfo::Controller::Human, "ARM", 0};
            params.players[1] = PlayerInfo{PlayerInfo::Controller::Computer, "CORE", 1};
            auto scene = std::make_unique<LoadingScene>(
                &vfs,
                &textureService,
                &audioService,
                &cursor,
                &graphics,
                &featureService,
                &*palette,
                &sceneManager,
                sdlContext,
                &sideDataMap,
                &viewportService,
                AudioService::LoopToken(),
                params);
            sceneManager.setNextScene(std::move(scene));
        }
        else
        {
            logger.info("Launching into the main menu");
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
                &viewportService,
                viewportService.width(),
                viewportService.height());
            sceneManager.setNextScene(std::move(scene));
        }

        logger.info("Entering main loop");
        sceneManager.execute();

        return 0;
    }
}

int main(int argc, char* argv[])
{
    auto localDataPath = rwe::getLocalDataPath();
    if (!localDataPath)
    {
        auto message = "Failed to determine local data path";
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", message, nullptr);
        return 1;
    }

    fs::path logPath(*localDataPath);
    logPath /= "rwe.log";
    auto logger = spdlog::basic_logger_mt("rwe", logPath.string(), true);
    logger->flush_on(spdlog::level::debug); // always flush

    try
    {
        boost::optional<std::string> mapName;
        if (argc == 2)
        {
            mapName = argv[1];
        }

        return rwe::run(*logger, *localDataPath, mapName);
    }
    catch (const std::runtime_error& e)
    {
        logger->critical(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
    catch (const std::logic_error& e)
    {
        logger->critical(e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
}
