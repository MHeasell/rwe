#include <GL/glew.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <rwe/AudioService.h>
#include <rwe/ColorPalette.h>
#include <rwe/GlobalConfig.h>
#include <rwe/LoadingScene.h>
#include <rwe/MainMenuScene.h>
#include <rwe/PathMapping.h>
#include <rwe/SceneContext.h>
#include <rwe/ShaderService.h>
#include <rwe/Viewport.h>
#include <rwe/config.h>
#include <rwe/game/PlayerColorIndex.h>
#include <rwe/io/gui/gui.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/ip_util.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/render/OpenGlVersion.h>
#include <rwe/rwe_time.h>
#include <rwe/scene/SceneManager.h>
#include <rwe/sdl/SdlContext.h>
#include <rwe/sdl/SdlContextManager.h>
#include <rwe/sim/Energy.h>
#include <rwe/sim/Metal.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/util.h>
#include <rwe/util/OpaqueArgs.h>
#include <rwe/util/Result.h>
#include <rwe/util/SimpleLogger.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>

namespace fs = std::filesystem;

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

    Result<SdlContext::GlContextUniquePtr, const char*> createOpenGlContext(SdlContext* sdlContext, SDL_Window* window, const OpenGlVersionInfo& requiredVersion)
    {
        LOG_INFO << "Requesting OpenGL version "
                 << requiredVersion.version.majorVersion << "."
                 << requiredVersion.version.minorVersion << ", "
                 << getOpenGlProfileName(requiredVersion.profile) << " profile";

        if (!sdlContext->glSetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, requiredVersion.version.majorVersion))
        {
            return Err(SDL_GetError());
        }
        if (!sdlContext->glSetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, requiredVersion.version.minorVersion))
        {
            return Err(SDL_GetError());
        }
        if (!sdlContext->glSetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, getSdlProfileMask(requiredVersion.profile)))
        {
            return Err(SDL_GetError());
        }
        if (!sdlContext->glSetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG))
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

    int run(const std::vector<fs::path>& searchPath, const PathMapping& pathMapping, const std::optional<GameParameters>& gameParameters, unsigned int desiredWindowWidth, unsigned int desiredWindowHeight, bool fullscreen, const std::string& imGuiIniPath, GlobalConfig& globalConfig)
    {
        LOG_INFO << ProjectNameVersion;
        LOG_INFO << "Current directory: " << fs::current_path().string();

        TimeService timeService(getTimestamp());

        LOG_INFO << "Initializing SDL";
        SdlContextManager sdlManager;

        // Set a reasonable number of audio channels
        // so that we always hear unit sounds.
        sdlManager.getSdlMixerContext()->allocateChannels(256);

        auto sdlContext = sdlManager.getSdlContext();

        // require a stencil buffer of some kind
        if (!sdlContext->glSetAttribute(SDL_GL_STENCIL_SIZE, 1))
        {
            throw std::runtime_error(SDL_GetError());
        }

        auto window = sdlContext->createWindow(
            "RWE",
            desiredWindowWidth,
            desiredWindowHeight,
            SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
        if (window == nullptr)
        {
            throw std::runtime_error(SDL_GetError());
        }

        if (fullscreen)
        {
            SDL_DisplayMode targetMode;
            auto displayID = sdlContext->getWindowDisplayIndex(window.get());
            if (displayID == 0)
            {
                throw std::runtime_error(SDL_GetError());
            }

            if (!sdlContext->getClosestDisplayMode(displayID, desiredWindowWidth, desiredWindowHeight, 0.0f, &targetMode))
            {
                throw std::runtime_error(SDL_GetError());
            }

            if (!sdlContext->setWindowDisplayMode(window.get(), &targetMode))
            {
                throw std::runtime_error(SDL_GetError());
            }
            sdlContext->setWindowSize(window.get(), desiredWindowWidth, desiredWindowHeight);
        }

        // Prevent the mouse from leaving the window.
        // We rely on nudging the edges of the screen to pan the camera,
        // so this is necessary for the game to work.
        sdlContext->setWindowGrab(window.get(), true);

        int windowWidth;
        int windowHeight;
        sdlContext->getWindowSize(window.get(), &windowWidth, &windowHeight);
        Viewport viewport(0, 0, windowWidth, windowHeight);

        LOG_INFO << "Initializing OpenGL context";

        auto glContextResult = createOpenGlContext(sdlContext, window.get(), OpenGlVersionInfo(3, 2, OpenGlProfile::Core));
        if (!glContextResult)
        {
            LOG_ERROR << "Failed to create preferred OpenGL context: " << glContextResult.getErr();
            glContextResult = createOpenGlContext(sdlContext, window.get(), OpenGlVersionInfo(3, 0, OpenGlProfile::Compatibility));
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
        LOG_INFO << "OpenGL version: " << glGetString(GL_VERSION);
        LOG_INFO << "OpenGL vendor: " << glGetString(GL_VENDOR);
        LOG_INFO << "OpenGL renderer: " << glGetString(GL_RENDERER);
        LOG_INFO << "OpenGL shading language version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
        LOG_DEBUG << "OpenGL extensions:";
        int openGlExtensionCount;
        glGetIntegerv(GL_NUM_EXTENSIONS, &openGlExtensionCount);
        for (int i = 0; i < openGlExtensionCount; ++i)
        {
            LOG_DEBUG << "  " << glGetStringi(GL_EXTENSIONS, i);
        }

        LOG_INFO << "Initializing Dear ImGui";
        ImGuiContext imGuiContext(imGuiIniPath, window.get(), glContext.get());

        LOG_INFO << "Initializing virtual file system";
        CompositeVirtualFileSystem vfs;
        for (const auto& path : searchPath)
        {
            addToVfs(vfs, path.string());
        }

        LOG_INFO << "Loading palette";
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

        LOG_INFO << "Loading GUI palette";
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

        LOG_INFO << "Initializing services";
        GraphicsContext graphics;
        graphics.enableCulling();
        graphics.enableBlending();

        ShaderService shaders = ShaderService::createShaderService(graphics);

        TextureService textureService(&graphics, &vfs, &*palette);

        AudioService audioService(sdlContext, sdlManager.getSdlMixerContext(), &vfs);

        // load sound definitions
        LOG_INFO << "Loading global sound definitions";
        auto allSoundBytes = vfs.readFile("gamedata/ALLSOUND.TDF");
        if (!allSoundBytes)
        {
            throw std::runtime_error("Couldn't read ALLSOUND.TDF");
        }

        std::string allSoundString(allSoundBytes->data(), allSoundBytes->size());
        auto allSoundTdf = parseTdfFromString(allSoundString);

        LOG_INFO << "Loading cursors";
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

        sdlContext->hideCursor();

        SceneManager sceneManager(sdlContext, window.get(), &graphics, &timeService, &imGuiContext, &cursor, &globalConfig, UiRenderService(&graphics, &shaders, &viewport), &viewport);

        LOG_INFO << "Loading side data";
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
            LOG_INFO << "Launching into game on map: " << gameParameters->mapName;
            auto scene = std::make_unique<LoadingScene>(
                sceneContext,
                &allSoundTdf,
                AudioService::LoopToken(),
                *gameParameters);
            sceneManager.setNextScene(std::shared_ptr<Scene>(std::move(scene)));
        }
        else
        {
            LOG_INFO << "Launching into the main menu";
            auto scene = std::make_unique<MainMenuScene>(
                sceneContext,
                &allSoundTdf,
                viewport.width(),
                viewport.height());
            sceneManager.setNextScene(std::shared_ptr<Scene>(std::move(scene)));
        }

        LOG_INFO << "Entering main loop";
        sceneManager.execute();

        LOG_INFO << "Finished main loop, exiting";

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

std::shared_ptr<rwe::SimpleLogger> createLogger(const fs::path& logFile)
{
    return std::make_shared<rwe::SimpleLogger>(logFile.string(), true);
}

std::shared_ptr<rwe::SimpleLogger> createLoggerInDir(const fs::path& logDir)
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
            return std::make_shared<rwe::SimpleLogger>(logPath.string(), true);
        }
        catch (const std::exception&)
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

        rwe::OpaqueArgs args;
        args.parse(argc, argv);
        args.parseConfig(configFilePath.string());

        if (args.isHelpRequested())
        {
            std::cout << "Usage: rwe [options]\n"
                      << "  --help                Show this message\n"
                      << "  --log <path>          Log output file path\n"
                      << "  --state-log <path>    Sim-state log file (desync debugging)\n"
                      << "  --width <pixels>      Window width (default: 800)\n"
                      << "  --height <pixels>     Window height (default: 600)\n"
                      << "  --fullscreen          Start in fullscreen mode\n"
                      << "  --interface-mode <m>  left-click or right-click (default: left-click)\n"
                      << "  --data-path <path>    Game data search path (repeatable)\n"
                      << "  --map <name>          Launch directly into a game on this map\n"
                      << "  --port <port>         Network port (default: 1337)\n"
                      << "  --player <spec>       Player spec: name;type;side;color (repeatable)\n"
                      << "  --dir-<name> <dir>    Override directory name for a data category\n"
                      << std::endl;
            return 0;
        }

        auto logger = args.contains("log") ? createLogger(fs::path(args.getString("log"))) : createLoggerInDir(*localDataPath);
        rwe::setGlobalLogger(logger);

        try
        {
            rwe::GlobalConfig config;
            config.leftClickInterfaceMode = args.getString("interface-mode", "left-click") != "right-click";
            std::optional<rwe::GameParameters> gameParameters;
            if (args.contains("map"))
            {
                const auto& mapName = args.getString("map");
                const auto& players = args.getMulti("player");

                gameParameters = rwe::GameParameters{mapName, 0};
                if (args.contains("state-log"))
                {
                    gameParameters->stateLogFile = args.getString("state-log");
                }
                gameParameters->localNetworkPort = args.getString("port", "1337");
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

            auto dataPaths = args.getMulti("data-path");
            if (!dataPaths.empty())
            {
                gameDataPaths.insert(gameDataPaths.end(), dataPaths.begin(), dataPaths.end());
            }
            else
            {
                gameDataPaths.emplace_back(*localDataPath) /= "Data";
            }

            auto screenWidth = args.getUint("width", 800);
            auto screenHeight = args.getUint("height", 600);
            auto fullscreen = args.getBool("fullscreen");

            auto pathMapping = constructDefaultPathMapping();
            pathMapping.ai = args.getString("dir-ai", "ai");
            pathMapping.anims = args.getString("dir-anims", "anims");
            pathMapping.bitmaps = args.getString("dir-bitmaps", "bitmaps");
            pathMapping.camps = args.getString("dir-camps", "camps");
            pathMapping.downloads = args.getString("dir-downloads", "downloads");
            pathMapping.fonts = args.getString("dir-fonts", "fonts");
            pathMapping.gamedata = args.getString("dir-gamedata", "gamedata");
            pathMapping.guis = args.getString("dir-guis", "guis");
            pathMapping.maps = args.getString("dir-maps", "maps");
            pathMapping.objects3d = args.getString("dir-objects3d", "objects3d");
            pathMapping.palettes = args.getString("dir-palettes", "palettes");
            pathMapping.scripts = args.getString("dir-scripts", "scripts");
            pathMapping.sounds = args.getString("dir-sounds", "sounds");
            pathMapping.textures = args.getString("dir-textures", "textures");
            pathMapping.unitpics = args.getString("dir-unitpics", "unitpics");
            pathMapping.units = args.getString("dir-units", "units");
            pathMapping.weapons = args.getString("dir-weapons", "weapons");

            return rwe::run(gameDataPaths, pathMapping, gameParameters, screenWidth, screenHeight, fullscreen, imGuiIniFilePath.string(), config);
        }
        catch (const std::exception& e)
        {
            LOG_CRITICAL << e.what();
            throw;
        }
    }
    catch (const std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", e.what(), nullptr);
        return 1;
    }
}
