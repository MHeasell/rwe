#include "MainMenuScene.h"
#include <rwe/LoadingScene.h>
#include <rwe/MainMenuModel.h>
#include <rwe/config.h>
#include <rwe/gui.h>
#include <rwe/ota.h>
#include <rwe/tdf.h>
#include <rwe/ui/UiSurface.h>

namespace rwe
{
    MainMenuScene::MainMenuScene(
        const SceneContext& sceneContext,
        TdfBlock* audioLookup,
        MapFeatureService* featureService,
        float width,
        float height)
        : sceneContext(sceneContext),
          soundLookup(audioLookup),
          featureService(featureService),
          scaledUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(640.0f, 480.0f)),
          nativeUiRenderService(sceneContext.graphics, sceneContext.shaders, UiCamera(width, height)),
          model(),
          uiFactory(sceneContext.textureService, sceneContext.audioService, soundLookup, sceneContext.vfs, &model, this),
          panelStack(),
          dialogStack(),
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
        panelStack.back()->render(scaledUiRenderService);

        for (auto& e : dialogStack)
        {
            scaledUiRenderService.fillScreen(Color(0, 0, 0, 63));
            e->render(scaledUiRenderService);
        }

        sceneContext.cursor->render(nativeUiRenderService);
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

        auto bgmName = bgmBlock->get().findValue("sound");
        if (!bgmName)
        {
            return AudioService::LoopToken();
        }

        auto bgm = sceneContext.audioService->loadSound(*bgmName);
        if (!bgm)
        {
            return AudioService::LoopToken();
        }

        return sceneContext.audioService->loopSound(*bgm);
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
        auto mainMenuGuiRaw = sceneContext.vfs->readFile("guis/MAINMENU.GUI");
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
        if (auto debugStrLabel = panel->find<UiLabel>("DebugString"))
        {
            debugStrLabel->get().setText(ProjectNameVersion);
            debugStrLabel->get().setAlignment(UiLabel::Alignment::Center);
        }
        goToMenu(std::move(panel));
    }

    void MainMenuScene::exit()
    {
        sceneContext.sceneManager->requestExit();
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
        auto mainMenuGuiRaw = sceneContext.vfs->readFile("guis/SINGLE.GUI");
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
        auto mainMenuGuiRaw = sceneContext.vfs->readFile("guis/SKIRMISH.GUI");
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
        if (auto mapLabel = panel->find<UiLabel>("MapName"))
        {
            auto sub = model.selectedMap.subscribe([&l = mapLabel->get()](const auto& selectedMap) {
                if (selectedMap)
                {
                    l.setText(selectedMap->name);
                }
                else
                {
                    l.setText("");
                }
            });
            mapLabel->get().addSubscription(std::move(sub));
        }

        goToMenu(std::move(panel));
    }

    void MainMenuScene::openMapSelectionDialog()
    {
        auto guiRaw = sceneContext.vfs->readFile("guis/SELMAP.GUI");
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

        if (auto descriptionLabel = panel->find<UiLabel>("DESCRIPTION"))
        {
            auto sub = model.candidateSelectedMap.subscribe([&l = descriptionLabel->get()](const auto& selectedMap) {
                if (selectedMap)
                {
                    l.setText(selectedMap->description);
                }
                else
                {
                    l.setText("");
                }
            });
            descriptionLabel->get().addSubscription(std::move(sub));
        }
        if (auto sizeLabel = panel->find<UiLabel>("SIZE"))
        {
            auto sub = model.candidateSelectedMap.subscribe([&l = sizeLabel->get()](const auto& selectedMap) {
                if (selectedMap)
                {
                    l.setText(selectedMap->size);
                }
                else
                {
                    l.setText("");
                }
            });
            sizeLabel->get().addSubscription(std::move(sub));
        }

        if (auto listBox = panel->find<UiListBox>("MAPNAMES"))
        {
            auto mapNames = getMapNames();
            for (const auto& e : mapNames)
            {
                listBox->get().appendItem(e);
            }

            auto sub = model.selectedMap.subscribe([&l = listBox->get()](const auto& selectedMap) {
                if (selectedMap)
                {
                    l.setSelectedItem(selectedMap->name);
                }
                else
                {
                    l.clearSelectedItem();
                }
            });
            listBox->get().addSubscription(std::move(sub));

            listBox->get().selectedIndex().subscribe([&l = listBox->get(), &c = *this](const auto& selectedMap) {
                if (selectedMap)
                {
                    c.setCandidateSelectedMap(l.getItems()[*selectedMap]);
                }
                else
                {
                    c.clearCandidateSelectedMap();
                }
            });
        }

        if (auto surface = panel->find<UiSurface>("MAPPIC"))
        {
            auto sub = model.candidateSelectedMap.subscribe([&s = surface->get()](const auto& info) {
                if (info)
                {
                    s.setBackground(info->minimap);
                }
                else
                {
                    s.clearBackground();
                }
            });
            surface->get().addSubscription(std::move(sub));
        }

        openDialog(std::move(panel));
    }

    void MainMenuScene::setCandidateSelectedMap(const std::string& mapName)
    {
        auto otaRaw = sceneContext.vfs->readFile(std::string("maps/").append(mapName).append(".ota"));
        if (!otaRaw)
        {
            return;
        }

        std::string otaStr(otaRaw->begin(), otaRaw->end());

        auto ota = parseOta(parseTdfFromString(otaStr));

        auto minimap = sceneContext.textureService->getMinimap(mapName);

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
        model.candidateSelectedMap.next(std::nullopt);
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
        for (unsigned int i = 1; i < 10; ++i)
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
        for (unsigned int i = 9; i >= 1; --i)
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
                player.teamIndex.next(std::nullopt);
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

    PlayerControllerType playerSettingsTypeToPlayerControllerType(MainMenuModel::PlayerSettings::Type t)
    {
        switch (t)
        {
            case MainMenuModel::PlayerSettings::Type::Human:
                return PlayerControllerTypeHuman();
            case MainMenuModel::PlayerSettings::Type::Computer:
                return PlayerControllerTypeComputer();
            default:
                throw std::logic_error("Invalid player settings type");
        }
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
                params.players[i] = std::nullopt;
                continue;
            }

            auto controller = playerSettingsTypeToPlayerControllerType(playerSlot.type.getValue());

            PlayerInfo playerInfo{controller, getSideName(playerSlot.side.getValue()), playerSlot.colorIndex.getValue()};
            params.players[i] = std::move(playerInfo);
        }

        auto scene = std::make_unique<LoadingScene>(
            sceneContext,
            featureService,
            std::move(bgm),
            params);

        sceneContext.sceneManager->setNextScene(std::move(scene));
    }

    Point MainMenuScene::toScaledCoordinates(int x, int y) const
    {
        auto clip = scaledUiRenderService.getCamera().screenToWorldRay(sceneContext.viewportService->toClipSpace(x, y));
        return Point(static_cast<int>(clip.origin.x), static_cast<int>(clip.origin.y));
    }

    std::vector<std::string> MainMenuScene::getMapNames()
    {
        auto mapNames = sceneContext.vfs->getFileNames("maps", ".ota");

        for (auto& e : mapNames)
        {
            // chop off the extension
            e.resize(e.size() - 4);
        }

        return mapNames;
    }
}
