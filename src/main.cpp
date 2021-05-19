#include <GL/glew.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/ColorPalette.h>
#include <rwe/GlobalConfig.h>
#include <rwe/LoadingScene.h>
#include <rwe/MainMenuScene.h>
#include <rwe/PathMapping.h>
#include <rwe/PlayerColorIndex.h>
#include <rwe/Result.h>
#include <rwe/SceneContext.h>
#include <rwe/SceneManager.h>
#include <rwe/SdlContextManager.h>
#include <rwe/ShaderService.h>
#include <rwe/Viewport.h>
#include <rwe/config.h>
#include <rwe/io/gui/gui.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/ip_util.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/render/OpenGlVersion.h>
#include <rwe/rwe_time.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/util.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <spdlog/spdlog.h>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace rwe
{
    OpenGlVersion getOpenGlContextVersion()
    {
        int major;
        int minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        return OpenGlVersion(major, minor);
    }

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

    int run(spdlog::logger& logger, const std::vector<fs::path>& searchPath, const PathMapping& pathMapping, const std::optional<GameParameters>& gameParameters, unsigned int desiredWindowWidth, unsigned int desiredWindowHeight, bool fullscreen, const std::string& imGuiIniPath, GlobalConfig& globalConfig)
    {
        logger.info(ProjectNameVersion);
        logger.info("Current directory: {0}", fs::current_path().string());

        TimeService timeService(getTimestamp());

        logger.info("Initializing SDL");
        SdlContextManager sdlManager;

        // Set a reasonable number of audio channels
        // so that we always hear unit sounds.
        sdlManager.getSdlMixerContext()->allocateChannels(256);

        auto sdlContext = sdlManager.getSdlContext();

        // require a stencil buffer of some kind
        if (sdlContext->glSetAttribute(SDL_GL_STENCIL_SIZE, 1) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto window = sdlContext->createWindow(
            "RWE",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            desiredWindowWidth,
            desiredWindowHeight,
            SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
        if (window == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        if (fullscreen)
        {
            SDL_DisplayMode desiredMode;
            desiredMode.w = desiredWindowWidth;
            desiredMode.h = desiredWindowHeight;
            desiredMode.refresh_rate = 0;
            desiredMode.format = 0;
            desiredMode.driverdata = 0;
            SDL_DisplayMode targetMode;
            auto displayIndex = sdlContext->getWindowDisplayIndex(window.get());
            if (displayIndex < 0)
            {
                throw std::runtime_error(SDL_GetError());
            }

            if (sdlContext->getClosestDisplayMode(displayIndex, &desiredMode, &targetMode) == nullptr)
            {
                throw std::runtime_error(SDL_GetError());
            }

            if (sdlContext->setWindowDisplayMode(window.get(), &targetMode) != 0)
            {
                throw std::runtime_error(SDL_GetError());
            }
            sdlContext->setWindowSize(window.get(), desiredWindowWidth, desiredWindowHeight);
        }

        // Prevent the mouse from leaving the window.
        // We rely on nudging the edges of the screen to pan the camera,
        // so this is necessary for the game to work.
        sdlContext->setWindowGrab(window.get(), SDL_TRUE);

        int windowWidth;
        int windowHeight;
        sdlContext->getWindowSize(window.get(), &windowWidth, &windowHeight);
        Viewport viewport(0, 0, windowWidth, windowHeight);

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

        logger.info("Initializing Dear ImGui");
        ImGuiContext imGuiContext(imGuiIniPath, window.get(), glContext.get());

        logger.info("Initializing virtual file system");
        CompositeVirtualFileSystem vfs;
        for (const auto& path : searchPath)
        {
            addToVfs(vfs, path.string());
        }

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
        Cursors cursors;
        cursors[*CursorType::Normal] = textureService.getGafEntry("anims/CURSORS.GAF", "cursornormal");
        cursors[*CursorType::Select] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorselect");
        cursors[*CursorType::Attack] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorattack");
        cursors[*CursorType::Move] = textureService.getGafEntry("anims/CURSORS.GAF", "cursormove");
        cursors[*CursorType::Guard] = textureService.getGafEntry("anims/CURSORS.GAF", "cursordefend");
        cursors[*CursorType::Repair] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorrepair");
        cursors[*CursorType::Red] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorred");
        cursors[*CursorType::Green] = textureService.getGafEntry("anims/CURSORS.GAF", "cursorgrn");
        CursorService cursor(sdlContext, &timeService, cursors);

        sdlContext->showCursor(SDL_DISABLE);

        SceneManager sceneManager(sdlContext, window.get(), &graphics, &timeService, &imGuiContext, &cursor, &globalConfig, UiRenderService(&graphics, &shaders, UiCamera(static_cast<float>(viewport.width()), static_cast<float>(viewport.height()))));

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

        SceneContext sceneContext(
            sdlContext,
            &viewport,
            &graphics,
            &textureService,
            &audioService,
            &cursor,
            &shaders,
            &vfs,
            &*palette,
            &*guiPalette,
            &sceneManager,
            &sideDataMap,
            &timeService,
            &pathMapping,
            &globalConfig);

        if (gameParameters)
        {
            logger.info("Launching into game on map: {0}", gameParameters->mapName);
            auto scene = std::make_unique<LoadingScene>(
                sceneContext,
                &allSoundTdf,
                AudioService::LoopToken(),
                *gameParameters);
            sceneManager.setNextScene(std::move(scene));
        }
        else
        {
            logger.info("Launching into the main menu");
            auto scene = std::make_unique<MainMenuScene>(
                sceneContext,
                &allSoundTdf,
                static_cast<float>(viewport.width()),
                static_cast<float>(viewport.height()));
            sceneManager.setNextScene(std::move(scene));
        }

        logger.info("Entering main loop");
        sceneManager.execute();

        logger.info("Finished main loop, exiting");

        return 0;
    }

    PlayerControllerType parseControllerFromString(const std::string& controllerString)
    {
        auto components = utf8Split(controllerString, ',');
        if (components.empty())
        {
            throw std::runtime_error("controller string was empty?");
        }

        if (components[0] == "Human")
        {
            return PlayerControllerTypeHuman();
        }

        if (components[0] == "Computer")
        {
            return PlayerControllerTypeComputer();
        }

        if (components[0] == "Network")
        {
            if (components.size() != 2)
            {
                throw std::runtime_error("Invalid network player string format");
            }
            auto hostAndPort = getHostAndPort(components[1]);
            if (!hostAndPort)
            {
                throw std::runtime_error("Invalid network player address format");
            }
            return PlayerControllerTypeNetwork{hostAndPort->first, hostAndPort->second};
        }

        throw std::runtime_error("Unknown controller string");
    }

    std::string parseSideFromString(const std::string& side)
    {
        if (side == "ARM" || side == "CORE")
        {
            return side;
        }

        throw std::runtime_error("Unknown side string");
    }

    PlayerColorIndex parseColorFromString(const std::string& colorString)
    {
        std::stringstream s(colorString);
        unsigned int i;
        s >> i;
        if (s.fail())
        {
            throw std::runtime_error("Invalid player colour string");
        }
        if (i > 9)
        {
            throw std::runtime_error("Invalid player colour string");
        }
        return PlayerColorIndex(i);
    }

    std::optional<PlayerInfo> parsePlayerInfoFromArg(const std::string& playerString)
    {
        if (playerString == "empty")
        {
            return std::nullopt;
        }

        auto components = rwe::utf8Split(playerString, ';');
        if (components.size() != 4)
        {
            throw std::runtime_error("Invalid player arg");
        }

        auto name = components[0];
        auto controller = parseControllerFromString(components[1]);
        auto side = parseSideFromString(components[2]);
        auto color = parseColorFromString(components[3]);

        return PlayerInfo{name, controller, side, color, Metal(1000), Energy(1000)};
    }
}

auto createLogger(const fs::path& logFile)
{
    return spdlog::basic_logger_mt("rwe", logFile.string(), true);
}

auto createLoggerInDir(const fs::path& logDir)
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

rwe::PathMapping constructDefaultPathMapping()
{
    rwe::PathMapping m;

    m.ai = "ai";
    m.anims = "anims";
    m.bitmaps = "bitmaps";
    m.camps = "camps";
    m.downloads = "downloads";
    m.features = "features";
    m.fonts = "fonts";
    m.gamedata = "gamedata";
    m.guis = "guis";
    m.maps = "maps";
    m.objects3d = "objects3d";
    m.palettes = "palettes";
    m.scripts = "scripts";
    m.sounds = "sounds";
    m.textures = "textures";
    m.unitpics = "unitpics";
    m.units = "units";
    m.weapons = "weapons";

    return m;
}

int main(int argc, char* argv[])
{
    try
    {
        auto localDataPath = rwe::getLocalDataPath();
        if (!localDataPath)
        {
            throw std::runtime_error("Failed to determine local data path");
        }

        fs::create_directories(*localDataPath);

        fs::path configFilePath(*localDataPath);
        configFilePath /= "rwe.cfg";

        fs::path imGuiIniFilePath(*localDataPath);
        imGuiIniFilePath /= "imgui.ini";

        po::options_description desc("Allowed options");

        // clang-format off
        desc.add_options()
            ("help", "produce help message")
            ("log", po::value<std::string>(), "Sets the log output file path")
            ("state-log", po::value<std::string>(), "Sets the output file for sim-state logs. This is a desync debugging feature.")
            ("width", po::value<unsigned int>()->default_value(800), "Sets the window width in pixels")
            ("height", po::value<unsigned int>()->default_value(600), "Sets the window height in pixels")
            ("fullscreen", po::bool_switch(), "Starts the application in fullscreen mode")
            ("interface-mode", po::value<std::string>()->default_value("left-click"), "left-click or right-click")
            ("data-path", po::value<std::vector<std::string>>(), "Sets the location(s) to search for game data")
            ("map", po::value<std::string>(), "If given, launches straight into a game on the given map")
            ("port", po::value<std::string>()->default_value("1337"), "Network port to bind to")
            ("player", po::value<std::vector<std::string>>(), "type;side;color")
            ("dir-ai", po::value<std::string>()->default_value("ai"), "AI directory name")
            ("dir-anims", po::value<std::string>()->default_value("anims"), "anims directory name")
            ("dir-bitmaps", po::value<std::string>()->default_value("bitmaps"), "bitmaps directory name")
            ("dir-camps", po::value<std::string>()->default_value("camps"), "campaigns directory name")
            ("dir-downloads", po::value<std::string>()->default_value("downloads"), "downloads directory name")
            ("dir-features", po::value<std::string>()->default_value("features"), "features directory name")
            ("dir-fonts", po::value<std::string>()->default_value("fonts"), "fonts directory name")
            ("dir-gamedata", po::value<std::string>()->default_value("gamedata"), "gamedata directory name")
            ("dir-guis", po::value<std::string>()->default_value("guis"), "GUIs directory name")
            ("dir-maps", po::value<std::string>()->default_value("maps"), "maps directory name")
            ("dir-objects3d", po::value<std::string>()->default_value("objects3d"), "3D objects directory name")
            ("dir-palettes", po::value<std::string>()->default_value("palettes"), "palettes directory name")
            ("dir-scripts", po::value<std::string>()->default_value("scripts"), "scripts directory name")
            ("dir-sounds", po::value<std::string>()->default_value("sounds"), "sounds directory name")
            ("dir-textures", po::value<std::string>()->default_value("textures"), "textures directory name")
            ("dir-unitpics", po::value<std::string>()->default_value("unitpics"), "unitpics directory name")
            ("dir-units", po::value<std::string>()->default_value("units"), "units directory name")
            ("dir-weapons", po::value<std::string>()->default_value("weapons"), "weapons directory name");
        // clang-format on

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        {
            std::ifstream configFileStream(configFilePath.string(), std::ios::binary);
            if (configFileStream.is_open())
            {
                po::store(po::parse_config_file(configFileStream, desc), vm);
            }
        }
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return 0;
        }

        auto logger = vm.count("log") ? createLogger(fs::path(vm["log"].as<std::string>())) : createLoggerInDir(*localDataPath);
        logger->set_level(spdlog::level::debug);
        logger->flush_on(spdlog::level::debug); // always flush

        try
        {
            rwe::GlobalConfig config;
            config.leftClickInterfaceMode = vm["interface-mode"].as<std::string>() != "right-click";
            std::optional<rwe::GameParameters> gameParameters;
            if (vm.count("map"))
            {
                const auto& mapName = vm["map"].as<std::string>();
                const auto& players = vm["player"].as<std::vector<std::string>>();

                gameParameters = rwe::GameParameters{mapName, 0};
                if (vm.count("state-log"))
                {
                    gameParameters->stateLogFile = vm["state-log"].as<std::string>();
                }
                gameParameters->localNetworkPort = vm["port"].as<std::string>();
                unsigned int playerIndex = 0;
                if (players.size() > 10)
                {
                    throw std::runtime_error("too many players");
                }
                for (const auto& playerString : players)
                {
                    gameParameters->players[playerIndex] = rwe::parsePlayerInfoFromArg(playerString);
                    ++playerIndex;
                }
            }

            std::vector<fs::path> gameDataPaths;

            if (vm.count("data-path"))
            {
                const auto& paths = vm["data-path"].as<std::vector<std::string>>();
                gameDataPaths.insert(gameDataPaths.end(), paths.begin(), paths.end());
            }
            else
            {
                gameDataPaths.emplace_back(*localDataPath) /= "Data";
            }

            auto screenWidth = vm["width"].as<unsigned int>();
            auto screenHeight = vm["height"].as<unsigned int>();
            auto fullscreen = vm["fullscreen"].as<bool>();

            auto pathMapping = constructDefaultPathMapping();
            pathMapping.ai = vm["dir-ai"].as<std::string>();
            pathMapping.anims = vm["dir-anims"].as<std::string>();
            pathMapping.bitmaps = vm["dir-bitmaps"].as<std::string>();
            pathMapping.camps = vm["dir-camps"].as<std::string>();
            pathMapping.downloads = vm["dir-features"].as<std::string>();
            pathMapping.fonts = vm["dir-fonts"].as<std::string>();
            pathMapping.gamedata = vm["dir-gamedata"].as<std::string>();
            pathMapping.guis = vm["dir-guis"].as<std::string>();
            pathMapping.maps = vm["dir-maps"].as<std::string>();
            pathMapping.objects3d = vm["dir-objects3d"].as<std::string>();
            pathMapping.palettes = vm["dir-palettes"].as<std::string>();
            pathMapping.scripts = vm["dir-scripts"].as<std::string>();
            pathMapping.sounds = vm["dir-sounds"].as<std::string>();
            pathMapping.textures = vm["dir-textures"].as<std::string>();
            pathMapping.unitpics = vm["dir-unitpics"].as<std::string>();
            pathMapping.units = vm["dir-units"].as<std::string>();
            pathMapping.weapons = vm["dir-weapons"].as<std::string>();

            return rwe::run(*logger, gameDataPaths, pathMapping, gameParameters, screenWidth, screenHeight, fullscreen, imGuiIniFilePath.string(), config);
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
