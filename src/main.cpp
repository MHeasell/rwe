#include <GL/glew.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/ColorPalette.h>
#include <rwe/GraphicsContext.h>
#include <rwe/LoadingScene.h>
#include <rwe/MainMenuScene.h>
#include <rwe/OpenGlVersion.h>
#include <rwe/Result.h>
#include <rwe/SceneManager.h>
#include <rwe/SdlContextManager.h>
#include <rwe/ShaderService.h>
#include <rwe/ViewportService.h>
#include <rwe/config.h>
#include <rwe/gui.h>
#include <rwe/tdf.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/util.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <spdlog/spdlog.h>

namespace fs = boost::filesystem;

namespace rwe
{
    /**
        Gets the major and minor OpenGL context version (for ex., OpenGL 3.1, where 3 is the major version, and 1 is the minor version).
    **/
    OpenGlVersion getOpenGlContextVersion()
    {
        int major;
        int minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        return OpenGlVersion(major, minor);
    }

    /**
        Initializes the OpenGL library (GLEW) to be used in the engine.
    **/
    void doGlewInit()
    {
        // Must set glewExperimental to true, otherwise in glew <= 1.13.0 glewInit() will fail
        // and set glError to GL_INVALID_ENUM if called in an OpenGL core context.
        // GL_INVALID_ENUM may *still* be emitted anyway, but isn't critical.
        // Ubuntu 16.04 and earlier uses glew 1.13.0, Ubuntu 18.04 uses glew 2.0.0.
        // See: https://www.khronos.org/opengl/wiki/OpenGL_Loading_Library
        glewExperimental = GL_TRUE;
        if (auto result = glewInit(); result != GLEW_OK)
        {
            throw std::runtime_error(reinterpret_cast<const char*>(glewGetErrorString(result)));
        }

        if (auto error = glGetError(); error == GL_NO_ERROR)
        {
            // all good, continue on
        }
        else if (error == GL_INVALID_ENUM)
        {
            // Ignore, expected from glew even with glewExperimental = true.
            // It should have still worked anyway.

            // Check if there are any more errors.
            if (auto nextError = glGetError(); nextError != GL_NO_ERROR)
            {
                throw OpenGlException(error);
            }
        }
        else
        {
            throw OpenGlException(error);
        }
    }

    /**
        Creates a context in which to store OpenGL objects.
        
        @param sdlContext The SDL context to be used in conjunction with OpenGL.
        @param window The window object to create an OpenGL context with.
        @param logger The logger object in which to record console output to.
        @param requiredVersion The specified OpenGL version that is required for usage.
    **/
    Result<SdlContext::GlContextUniquePtr, const char*> createOpenGlContext(SdlContext* sdlContext, SDL_Window* window, spdlog::logger& logger, const OpenGlVersionInfo& requiredVersion)
    {
        logger.info(
            "Requesting OpenGL version {0}.{1}, {2} profile",
            requiredVersion.version.majorVersion,
            requiredVersion.version.minorVersion,
            getOpenGlProfileName(requiredVersion.profile));

        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, requiredVersion.version.majorVersion) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, requiredVersion.version.minorVersion) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, getSdlProfileMask(requiredVersion.profile)) != 0)
        {
            return Err(SDL_GetError());
        }
        if (sdlContext->glSetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) != 0)
        {
            return Err(SDL_GetError());
        }

        auto glContext = sdlContext->glCreateContext(window);
        if (glContext == nullptr)
        {
            return Err(SDL_GetError());
        }

        auto contextVersion = getOpenGlContextVersion();
        if (contextVersion < requiredVersion.version)
        {
            static const char* errMessage = "Created OpenGL context did not meet version requirements";
            return Err(errMessage);
        }

        return Ok(std::move(glContext));
    };

    /**
        Does the actual execution of the engine when the program is ran. 
        Establishes the window size, sdlContextManager, openGL context, initializes GLEW, and so on.
        
        @param logger The logger in which to print output to (for debugging, errors, etc.)
        @param localDataPath The path in which to retrieve TA data  (.hpi, .ufo, rev31.gp3, etc.)
        @param mapName (???)
    **/
    int run(spdlog::logger& logger, const fs::path& localDataPath, const std::optional<std::string>& mapName)
    {
        logger.info(ProjectNameVersion);
        logger.info("Current directory: {0}", fs::current_path().string());

        ViewportService viewportService(800, 600);

        logger.info("Initializing SDL");
        SdlContextManager sdlManager;

        auto sdlContext = sdlManager.getSdlContext();

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

        logger.info("Initializing OpenGL context");

        auto glContextResult = createOpenGlContext(sdlContext, window.get(), logger, OpenGlVersionInfo(3, 2, OpenGlProfile::Core));
        if (!glContextResult)
        {
            logger.error("Failed to create preferred OpenGL context: {0}", glContextResult.getErr());
            glContextResult = createOpenGlContext(sdlContext, window.get(), logger, OpenGlVersionInfo(3, 0, OpenGlProfile::Compatibility));
            if (!glContextResult)
            {
                throw std::runtime_error(glContextResult.getErr());
            }
        }

        auto glContext = std::move(*glContextResult);
        if (glContext == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        doGlewInit();

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

        logger.info("Loading GUI palette");
        auto guiPaletteBytes = vfs.readFile("palettes/GUIPAL.PAL");
        if (!guiPaletteBytes)
        {
            throw std::runtime_error("Couldn't find palette");
        }

        auto guiPalette = readPalette(*guiPaletteBytes);
        if (!guiPalette)
        {
            throw std::runtime_error("Couldn't read GUI palette");
        }

        logger.info("Initializing services");
        GraphicsContext graphics;
        graphics.enableCulling();
        graphics.enableBlending();

        ShaderService shaders = ShaderService::createShaderService(graphics);

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
            textureService.getGafEntry("anims/CURSORS.GAF", "cursorselect"),
            textureService.getGafEntry("anims/CURSORS.GAF", "cursorattack"),
            textureService.getGafEntry("anims/CURSORS.GAF", "cursorred"));

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
                &shaders,
                &featureService,
                &*palette,
                &*guiPalette,
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
                &shaders,
                &featureService,
                &*palette,
                &*guiPalette,
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

        logger.info("Finished main loop, exiting");
        return 0;
    }
}

/**
    Creates a logger. 
    
    @param path The path to get to the log.
    @param logDir The directory in which to place the log.
**/
auto createLogger(const fs::path& logDir)
{
    for (int i = 0; i < 3; ++i)
    {
        fs::path logPath(logDir);
        if (i == 0)
        {
            logPath /= "rwe.log";
        }
        else
        {
            logPath /= "rwe" + std::to_string(i) + ".log";
        }

        try
        {
            return spdlog::basic_logger_mt("rwe", logPath.string(), true);
        }
        catch (const spdlog::spdlog_ex& ex)
        {
        }
    }

    throw std::runtime_error("Failed to create logger");
}

/**
    The main method where the program starts. Initializes logs and starts up the RWE engine.
**/
int main(int argc, char* argv[])
{
    try
    {
        auto localDataPath = rwe::getLocalDataPath();
        if (!localDataPath)
        {
            throw std::runtime_error("Failed to determine local data path");
        }

        fs::path logPath(*localDataPath);
        auto logger = createLogger(logPath);
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug); // always flush

        try
        {
            std::optional<std::string> mapName;
            if (argc == 2)
            {
                mapName = argv[1];
            }

            return rwe::run(*logger, *localDataPath, mapName);
        }
        catch (const std::exception& e)
        {
            logger->critical(e.what());
            throw;
        }
    }
    catch (const std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
}
