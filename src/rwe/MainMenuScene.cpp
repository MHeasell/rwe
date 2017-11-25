#include "LoadingScene.h"
#include "MainMenuModel.h"
#include "MainMenuScene.h"
#include "ota.h"
#include "tdf.h"

#include <rwe/gui.h>

namespace rwe
{
    MainMenuScene::MainMenuScene(
        SceneManager* sceneManager,
        AbstractVirtualFileSystem* vfs,
        TextureService* textureService,
        AudioService* audioService,
        TdfBlock* audioLookup,
        GraphicsContext* graphics,
        RenderService* renderService,
        MapFeatureService* featureService,
        const ColorPalette* palette,
        CursorService* cursor,
        SdlContext* sdl,
        const std::unordered_map<std::string, SideData>* sideData,
        ViewportService* viewportService,
        float width,
        float height)
        : sceneManager(sceneManager),
          vfs(vfs),
          textureService(textureService),
          audioService(audioService),
          soundLookup(audioLookup),
          graphics(graphics),
          renderService(renderService),
          featureService(featureService),
          palette(palette),
          cursor(cursor),
          sdl(sdl),
          sideData(sideData),
          viewportService(viewportService),
          model(),
          uiFactory(textureService, audioService, soundLookup, vfs, &model, this),
          panelStack(),
          dialogStack(),
          scaledUiCamera(640, 480),
          nativeUiCamera(width, height),
          bgm()
    {
    }

    void MainMenuScene::init()
    {
        bgm = startBgm();
        goToMainMenu();
    }

    void MainMenuScene::render(GraphicsContext& context)
    {
        context.applyCamera(scaledUiCamera);
        panelStack.back()->render(context);

        for (auto& e : dialogStack)
        {
            context.fillColor(0.0f, 0.0f, scaledUiCamera.getWidth(), scaledUiCamera.getHeight(), Color(0, 0, 0, 63));
            e->render(context);
        }

        context.applyCamera(nativeUiCamera);
        cursor->render(context);
    }

    void MainMenuScene::onMouseDown(MouseButtonEvent event)
    {
        auto p = toScaledCoordinates(event.x, event.y);
        event.x = p.x;
        event.y = p.y;
        topPanel().mouseDown(event);
    }

    void MainMenuScene::onMouseUp(MouseButtonEvent event)
    {
        auto p = toScaledCoordinates(event.x, event.y);
        event.x = p.x;
        event.y = p.y;
        topPanel().mouseUp(event);
    }

    void MainMenuScene::onMouseMove(MouseMoveEvent event)
    {
        auto p = toScaledCoordinates(event.x, event.y);
        event.x = p.x;
        event.y = p.y;
        topPanel().mouseMove(event);
    }

    AudioService::LoopToken MainMenuScene::startBgm()
    {
        auto bgmBlock = soundLookup->findBlock("BGM");
        if (!bgmBlock)
        {
            return AudioService::LoopToken();
        }

        auto bgmName = bgmBlock->findValue("sound");
        if (!bgmName)
        {
            return AudioService::LoopToken();
        }

        auto bgm = audioService->loadSound(*bgmName);
        if (!bgm)
        {
            return AudioService::LoopToken();
        }

        return audioService->loopSound(*bgm);
    }

    void MainMenuScene::goToPreviousMenu()
    {
        if (!dialogStack.empty())
        {
            dialogStack.pop_back();
            return;
        }

        if (panelStack.size() > 1)
        {
            panelStack.pop_back();
            return;
        }

        exit();
    }

    void MainMenuScene::goToMenu(std::unique_ptr<UiPanel>&& panel)
    {
        panelStack.push_back(std::move(panel));
    }

    void MainMenuScene::openDialog(std::unique_ptr<UiPanel>&& panel)
    {
        dialogStack.push_back(std::move(panel));
    }

    UiPanel& MainMenuScene::topPanel()
    {
        if (!dialogStack.empty())
        {
            return *(dialogStack.back());
        }

        return *(panelStack.back());
    }

    void MainMenuScene::update()
    {
        topPanel().update(static_cast<float>(SceneManager::TickInterval) / 1000.0f);
    }

    void MainMenuScene::onMouseWheel(MouseWheelEvent event)
    {
        topPanel().mouseWheel(event);
    }

    void MainMenuScene::onKeyDown(const SDL_Keysym& keysym)
    {
        topPanel().keyDown(KeyEvent(keysym.sym));
    }

    void MainMenuScene::goToMainMenu()
    {
        auto mainMenuGuiRaw = vfs->readFile("guis/MAINMENU.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read MAINMENU.GUI");
        }

        std::string gui(mainMenuGuiRaw->data(), mainMenuGuiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("MAINMENU", "FrontendX", *parsedGui);
        goToMenu(std::move(panel));
    }

    void MainMenuScene::exit()
    {
        sceneManager->requestExit();
    }

    void MainMenuScene::message(const std::string& topic, const std::string& message)
    {
        if (message == "PrevMenu" || message == "PREVMENU")
        {
            goToPreviousMenu();
        }
        else if (topic == "MAINMENU")
        {
            if (message == "EXIT")
            {
                exit();
            }
            else if (message == "SINGLE")
            {
                goToSingleMenu();
            }
        }
        else if (topic == "SINGLE")
        {
            if (message == "Skirmish")
            {
                goToSkirmishMenu();
            }
        }
        else if (topic == "SKIRMISH")
        {
            if (message == "SelectMap")
            {
                resetCandidateSelectedMap();
                openMapSelectionDialog();
            }
            else if (message == "Start")
            {
                startGame();
            }
        }
        else if (topic == "SELMAP")
        {
            if (message == "LOAD")
            {
                commitSelectedMap();
                goToPreviousMenu();
            }
        }
    }

    void MainMenuScene::goToSingleMenu()
    {
        auto mainMenuGuiRaw = vfs->readFile("guis/SINGLE.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read SINGLE.GUI");
        }

        std::string gui(mainMenuGuiRaw->data(), mainMenuGuiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("SINGLE", "SINGLEBG", *parsedGui);
        goToMenu(std::move(panel));
    }

    void MainMenuScene::goToSkirmishMenu()
    {
        auto mainMenuGuiRaw = vfs->readFile("guis/SKIRMISH.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read SKIRMISH.GUI");
        }

        std::string gui(mainMenuGuiRaw->data(), mainMenuGuiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("SKIRMISH", "Skirmsetup4x", *parsedGui);
        goToMenu(std::move(panel));
    }

    void MainMenuScene::openMapSelectionDialog()
    {
        auto guiRaw = vfs->readFile("guis/SELMAP.GUI");
        if (!guiRaw)
        {
            throw std::runtime_error("Couldn't read SELMAP.GUI");
        }

        std::string gui(guiRaw->data(), guiRaw->size());
        auto parsedGui = parseGui(parseTdfFromString(gui));
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("SELMAP", "DSelectmap2", *parsedGui);
        openDialog(std::move(panel));
    }

    void MainMenuScene::setCandidateSelectedMap(const std::string& mapName)
    {
        auto otaRaw = vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            return;
        }

        std::string otaStr(otaRaw->begin(), otaRaw->end());

        auto ota = parseOta(parseTdfFromString(otaStr));

        auto minimap = textureService->getMinimap(mapName);

        // this is what TA shows in its map selection dialog
        auto sizeInfo = std::string().append(ota.memory).append("  Players: ").append(ota.numPlayers);

        MainMenuModel::SelectedMapInfo info(
            mapName,
            ota.missionDescription,
            sizeInfo,
            minimap);

        model.candidateSelectedMap.next(std::move(info));
    }

    void MainMenuScene::resetCandidateSelectedMap()
    {
        model.candidateSelectedMap.next(model.selectedMap.getValue());
    }

    void MainMenuScene::commitSelectedMap()
    {
        model.selectedMap.next(model.candidateSelectedMap.getValue());
    }

    void MainMenuScene::clearCandidateSelectedMap()
    {
        model.candidateSelectedMap.next(boost::none);
    }

    void MainMenuScene::scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollUpMessage m;
        model.groupMessages.next(GroupMessage(topic, group, name, m));
    }

    void MainMenuScene::scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollDownMessage m;
        model.groupMessages.next(GroupMessage(topic, group, name, m));
    }

    void MainMenuScene::scrollMessage(
        const std::string& topic,
        unsigned int group,
        const std::string& name,
        const ScrollPositionMessage& message)
    {
        GroupMessage gm(topic, group, name, message);
        model.groupMessages.next(gm);
    }

    void MainMenuScene::incrementPlayerMetal(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.metal.getValue() == 200)
        {
            player.metal.next(500);
        }
        else if (player.metal.getValue() < 10000)
        {
            player.metal.next(player.metal.getValue() + 500);
        }
    }

    void MainMenuScene::decrementPlayerMetal(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.metal.getValue() > 500)
        {
            player.metal.next(player.metal.getValue() - 500);
        }
        else if (player.metal.getValue() == 500)
        {
            player.metal.next(200);
        }
    }

    void MainMenuScene::incrementPlayerEnergy(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.energy.getValue() == 200)
        {
            player.energy.next(500);
        }
        else if (player.energy.getValue() < 10000)
        {
            player.energy.next(player.energy.getValue() + 500);
        }
    }

    void MainMenuScene::decrementPlayerEnergy(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.energy.getValue() > 500)
        {
            player.energy.next(player.energy.getValue() - 500);
        }
        else if (player.energy.getValue() == 500)
        {
            player.energy.next(200);
        }
    }

    void MainMenuScene::togglePlayer(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        switch (player.type.getValue())
        {
            case MainMenuModel::PlayerSettings::Type::Open:
            {
                auto color = player.colorIndex.getValue();
                if (model.isColorInUse(color))
                {
                    auto newColor = model.getFirstFreeColor();
                    if (!newColor)
                    {
                        throw std::logic_error("No free colors for the player");
                    }
                    color = *newColor;
                }

                bool humanAllowed = std::find_if(model.players.begin(), model.players.end(), [](const auto& p) {
                    return p.type.getValue() == MainMenuModel::PlayerSettings::Type::Human;
                }) == model.players.end();
                if (humanAllowed)
                {
                    player.type.next(MainMenuModel::PlayerSettings::Type::Human);
                }
                else
                {
                    player.type.next(MainMenuModel::PlayerSettings::Type::Computer);
                }
                player.colorIndex.next(color);
                break;
            }
            case MainMenuModel::PlayerSettings::Type::Human:
                player.type.next(MainMenuModel::PlayerSettings::Type::Computer);
                break;
            case MainMenuModel::PlayerSettings::Type::Computer:
                player.type.next(MainMenuModel::PlayerSettings::Type::Open);
                break;
        }
    }

    void MainMenuScene::togglePlayerSide(int playerIndex)
    {
        auto& player = model.players[playerIndex];
        switch (player.side.getValue())
        {
            case MainMenuModel::PlayerSettings::Side::Arm:
                player.side.next(MainMenuModel::PlayerSettings::Side::Core);
                break;
            case MainMenuModel::PlayerSettings::Side::Core:
                player.side.next(MainMenuModel::PlayerSettings::Side::Arm);
                break;
        }
    }

    void MainMenuScene::cyclePlayerColor(int playerIndex)
    {
        auto& player = model.players[playerIndex];
        auto currentColor = player.colorIndex.getValue();
        for (int i = 1; i < 10; ++i)
        {
            auto newColor = (currentColor + i) % 10;
            if (!model.isColorInUse(newColor))
            {
                player.colorIndex.next(newColor);
                return;
            }
        }
    }

    void MainMenuScene::reverseCyclePlayerColor(int playerIndex)
    {
        auto& player = model.players[playerIndex];
        auto currentColor = player.colorIndex.getValue();
        for (int i = 9; i >= 1; --i)
        {
            auto newColor = (currentColor + i) % 10;
            if (!model.isColorInUse(newColor))
            {
                player.colorIndex.next(newColor);
                return;
            }
        }
    }

    void MainMenuScene::cyclePlayerTeam(int playerIndex)
    {
        auto& player = model.players[playerIndex];
        if (!player.teamIndex.getValue())
        {
            player.teamIndex.next(0);
            model.teamChanges.next(0);
        }
        else
        {
            auto val = *(player.teamIndex.getValue());
            if (val == 4)
            {
                player.teamIndex.next(boost::none);
                model.teamChanges.next(val);
            }
            else
            {
                player.teamIndex.next(val + 1);
                model.teamChanges.next(val);
                model.teamChanges.next(val + 1);
            }
        }
    }

    std::string getSideName(const MainMenuModel::PlayerSettings::Side side)
    {
        switch (side)
        {
            case MainMenuModel::PlayerSettings::Side::Arm:
                return "ARM";
            case MainMenuModel::PlayerSettings::Side::Core:
                return "CORE";
        }

        throw std::logic_error("Invalid side");
    }

    void MainMenuScene::startGame()
    {
        if (!model.selectedMap.getValue())
        {
            return;
        }

        GameParameters params{model.selectedMap.getValue()->name, 0};

        for (unsigned int i = 0; i < model.players.size(); ++i)
        {
            const auto& playerSlot = model.players[i];
            if (playerSlot.type.getValue() == MainMenuModel::PlayerSettings::Type::Open)
            {
                params.players[i] = boost::none;
                continue;
            }

            PlayerInfo::Controller controller;
            switch (playerSlot.type.getValue())
            {
                case MainMenuModel::PlayerSettings::Type::Human:
                    controller = PlayerInfo::Controller::Human;
                    break;
                case MainMenuModel::PlayerSettings::Type::Computer:
                    controller = PlayerInfo::Controller::Computer;
                    break;
                default:
                    throw std::logic_error("Invalid slot type");
            }

            PlayerInfo playerInfo{controller, getSideName(playerSlot.side.getValue()), playerSlot.colorIndex.getValue()};
            params.players[i] = std::move(playerInfo);
        }

        auto scene = std::make_unique<LoadingScene>(
            vfs,
            textureService,
            audioService,
            cursor,
            graphics,
            renderService,
            featureService,
            palette,
            sceneManager,
            sdl,
            sideData,
            viewportService,
            std::move(bgm),
            params);

        sceneManager->setNextScene(std::move(scene));
    }

    Point MainMenuScene::toScaledCoordinates(int x, int y) const
    {
        auto clip = scaledUiCamera.screenToWorldRay(viewportService->toClipSpace(x, y));
        return Point(static_cast<int>(clip.origin.x), static_cast<int>(clip.origin.y));
    }
}
