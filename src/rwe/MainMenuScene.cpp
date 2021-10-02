#include "MainMenuScene.h"
#include <rwe/Index.h>
#include <rwe/LoadingScene.h>
#include <rwe/MainMenuModel.h>
#include <rwe/camera_util.h>
#include <rwe/config.h>
#include <rwe/io/gui/gui.h>
#include <rwe/io/ota/ota.h>
#include <rwe/io/tdf/tdf.h>
#include <rwe/resource_io.h>
#include <rwe/ui/UiSurface.h>

namespace rwe
{
    const Viewport MainMenuViewport(0, 0, 640, 480);

    MainMenuScene::MainMenuScene(
        const SceneContext& sceneContext,
        TdfBlock* audioLookup,
        float width,
        float height)
        : sceneContext(sceneContext),
          soundLookup(audioLookup),
          scaledUiRenderService(sceneContext.graphics, sceneContext.shaders, &MainMenuViewport),
          nativeUiRenderService(sceneContext.graphics, sceneContext.shaders, sceneContext.viewport),
          model(),
          uiFactory(sceneContext.textureService, sceneContext.audioService, soundLookup, sceneContext.vfs, sceneContext.pathMapping, 640, 480),
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

    void MainMenuScene::render()
    {
        panelStack.back()->render(scaledUiRenderService);

        for (auto& e : dialogStack)
        {
            scaledUiRenderService.fillScreen(Color(0, 0, 0, 63));
            e->render(scaledUiRenderService);
        }
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
        auto& p = panelStack.emplace_back(std::move(panel));
        p->groupMessages().subscribe([this](const auto& msg) {
            if (auto activate = std::get_if<ActivateMessage>(&msg.message); activate != nullptr)
            {
                message(msg.topic, msg.controlName, *activate);
            }
        });
    }

    void MainMenuScene::openDialog(std::unique_ptr<UiPanel>&& panel)
    {
        auto& p = dialogStack.emplace_back(std::move(panel));
        p->groupMessages().subscribe([this](const auto& msg) {
            if (auto activate = std::get_if<ActivateMessage>(&msg.message); activate != nullptr)
            {
                message(msg.topic, msg.controlName, *activate);
            }
        });
    }

    UiPanel& MainMenuScene::topPanel()
    {
        if (!dialogStack.empty())
        {
            return *(dialogStack.back());
        }

        return *(panelStack.back());
    }

    void MainMenuScene::update(int millisecondsElapsed)
    {
        topPanel().update(static_cast<float>(millisecondsElapsed) / 1000.0f);
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
        auto mainMenuGuiRaw = sceneContext.vfs->readFile(sceneContext.pathMapping->guis + "/MAINMENU.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read MAINMENU.GUI");
        }

        auto parsedGui = parseGuiFromBytes(*mainMenuGuiRaw);
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

    std::optional<int> matchesPlayer(const std::string& format, const std::string& input)
    {
        // FIXME: this should probably be a regex match instead of this crude brute-force search
        for (int i = 0; i < 10; ++i)
        {
            if (input == fmt::format(format, i))
            {
                return i;
            }
        }

        return std::nullopt;
    }

    void MainMenuScene::message(const std::string& topic, const std::string& message, const ActivateMessage& details)
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
            else if (auto num = matchesPlayer("PLAYER{0}", message))
            {
                togglePlayer(*num);
            }
            else if (auto num = matchesPlayer("PLAYER{0}_side", message))
            {
                togglePlayerSide(*num);
            }
            else if (auto num = matchesPlayer("PLAYER{0}_color", message))
            {
                switch (details.type)
                {
                    case ActivateMessage::Type::Primary:
                        cyclePlayerColor(*num);
                        break;
                    case ActivateMessage::Type::Secondary:
                        reverseCyclePlayerColor(*num);
                        break;
                    default:
                        throw std::logic_error("Invalid activation type");
                }
            }
            else if (auto num = matchesPlayer("PLAYER{0}_team", message))
            {
                cyclePlayerTeam(*num);
            }
            else if (auto num = matchesPlayer("PLAYER{0}_metal", message))
            {
                switch (details.type)
                {
                    case ActivateMessage::Type::Primary:
                        incrementPlayerMetal(*num);
                        break;
                    case ActivateMessage::Type::Secondary:
                        decrementPlayerMetal(*num);
                        break;
                    default:
                        throw std::logic_error("Invalid activation type");
                }
            }
            else if (auto num = matchesPlayer("PLAYER{0}_energy", message))
            {
                switch (details.type)
                {
                    case ActivateMessage::Type::Primary:
                        incrementPlayerEnergy(*num);
                        break;
                    case ActivateMessage::Type::Secondary:
                        decrementPlayerEnergy(*num);
                        break;
                    default:
                        throw std::logic_error("Invalid activation type");
                }
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
        auto mainMenuGuiRaw = sceneContext.vfs->readFile(sceneContext.pathMapping->guis + "/SINGLE.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read SINGLE.GUI");
        }

        auto parsedGui = parseGuiFromBytes(*mainMenuGuiRaw);
        if (!parsedGui)
        {
            throw std::runtime_error("Failed to parse GUI file");
        }

        auto panel = uiFactory.panelFromGuiFile("SINGLE", "SINGLEBG", *parsedGui);
        goToMenu(std::move(panel));
    }

    void MainMenuScene::goToSkirmishMenu()
    {
        auto mainMenuGuiRaw = sceneContext.vfs->readFile(sceneContext.pathMapping->guis + "/SKIRMISH.GUI");
        if (!mainMenuGuiRaw)
        {
            throw std::runtime_error("Couldn't read SKIRMISH.GUI");
        }

        auto parsedGui = parseGuiFromBytes(*mainMenuGuiRaw);
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

        attachPlayerSelectionComponents("SKIRMISH", *panel);

        goToMenu(std::move(panel));
    }

    void MainMenuScene::openMapSelectionDialog()
    {
        auto guiRaw = sceneContext.vfs->readFile(sceneContext.pathMapping->guis + "/SELMAP.GUI");
        if (!guiRaw)
        {
            throw std::runtime_error("Couldn't read SELMAP.GUI");
        }

        auto parsedGui = parseGuiFromBytes(*guiRaw);
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

    void MainMenuScene::incrementPlayerMetal(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.metal.getValue() == Metal(200))
        {
            player.metal.next(Metal(500));
        }
        else if (player.metal.getValue() < Metal(10000))
        {
            player.metal.next(player.metal.getValue() + Metal(500));
        }
    }

    void MainMenuScene::decrementPlayerMetal(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.metal.getValue() > Metal(500))
        {
            player.metal.next(player.metal.getValue() - Metal(500));
        }
        else if (player.metal.getValue() == Metal(500))
        {
            player.metal.next(Metal(200));
        }
    }

    void MainMenuScene::incrementPlayerEnergy(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.energy.getValue() == Energy(200))
        {
            player.energy.next(Energy(500));
        }
        else if (player.energy.getValue() < Energy(10000))
        {
            player.energy.next(player.energy.getValue() + Energy(500));
        }
    }

    void MainMenuScene::decrementPlayerEnergy(int playerIndex)
    {
        auto& player = model.players[playerIndex];

        if (player.energy.getValue() > Energy(500))
        {
            player.energy.next(player.energy.getValue() - Energy(500));
        }
        else if (player.energy.getValue() == Energy(500))
        {
            player.energy.next(Energy(200));
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
            auto newColor = PlayerColorIndex((currentColor.value + i) % 10);
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
            auto newColor = PlayerColorIndex((currentColor.value + i) % 10);
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

        for (Index i = 0; i < getSize(model.players); ++i)
        {
            const auto& playerSlot = model.players[i];
            if (playerSlot.type.getValue() == MainMenuModel::PlayerSettings::Type::Open)
            {
                params.players[i] = std::nullopt;
                continue;
            }

            auto controller = playerSettingsTypeToPlayerControllerType(playerSlot.type.getValue());

            PlayerInfo playerInfo{std::nullopt, controller, getSideName(playerSlot.side.getValue()), playerSlot.colorIndex.getValue(), playerSlot.metal.getValue(), playerSlot.energy.getValue()};
            params.players[i] = std::move(playerInfo);
        }

        auto scene = std::make_unique<LoadingScene>(
            sceneContext,
            soundLookup,
            std::move(bgm),
            params);

        sceneContext.sceneManager->setNextScene(std::move(scene));
    }

    Point MainMenuScene::toScaledCoordinates(int x, int y) const
    {
        auto clip = screenToWorldRayUtil(scaledUiRenderService.getInverseViewProjectionMatrix(), sceneContext.viewport->toClipSpace(x, y));
        return Point(static_cast<int>(clip.origin.x), static_cast<int>(clip.origin.y));
    }

    std::vector<std::string> MainMenuScene::getMapNames()
    {
        auto mapNames = sceneContext.vfs->getFileNames("maps", ".ota");

        // Keep only maps that have a multiplayer schema
        mapNames.erase(std::remove_if(mapNames.begin(), mapNames.end(), [this](const auto& e) { return !hasMultiplayerSchema(e); }), mapNames.end());

        for (auto& e : mapNames)
        {
            // chop off the extension
            e.resize(e.size() - 4);
        }

        return mapNames;
    }

    void MainMenuScene::attachPlayerSelectionComponents(const std::string& guiName, UiPanel& panel)
    {
        unsigned int tableStart = 78;
        unsigned int rowHeight = 20;

        for (int i = 0; i < 10; ++i)
        {
            unsigned int rowStart = tableStart + (i * rowHeight);

            {
                // player name button
                unsigned int width = 112;
                unsigned int height = 20;

                auto b = uiFactory.createBasicButton(45, rowStart, width, height, guiName, "skirmname", "Player");
                b->setName("PLAYER" + std::to_string(i));
                b->setTextAlign(UiStagedButton::TextAlign::Center);

                auto sub = model.players[i].type.subscribe([b = b.get(), &panel, this, guiName, i](MainMenuModel::PlayerSettings::Type type) {
                    switch (type)
                    {
                        case MainMenuModel::PlayerSettings::Type::Open:
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            b->setLabel("Open");
                            break;
                        case MainMenuModel::PlayerSettings::Type::Human:
                            b->setLabel("Player");
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            attachDetailedPlayerSelectionComponents(guiName, panel, i);
                            break;
                        case MainMenuModel::PlayerSettings::Type::Computer:
                            b->setLabel("Computer");
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            attachDetailedPlayerSelectionComponents(guiName, panel, i);
                            break;
                    }
                });
                b->addSubscription(std::move(sub));

                panel.appendChild(std::move(b));
            }
        }
    }

    void MainMenuScene::attachDetailedPlayerSelectionComponents(const std::string& guiName, UiPanel& panel, int i)
    {
        unsigned int tableStart = 78;
        unsigned int rowHeight = 20;

        unsigned int rowStart = tableStart + (i * rowHeight);

        {
            // side button
            unsigned int width = 44;
            unsigned int height = 20;

            auto b = uiFactory.createStagedButton(163, rowStart, width, height, guiName, "SIDEx", std::vector<std::string>(2), 2);
            b->setName("PLAYER" + std::to_string(i) + "_side");

            auto sub = model.players[i].side.subscribe([b = b.get()](MainMenuModel::PlayerSettings::Side side) {
                switch (side)
                {
                    case MainMenuModel::PlayerSettings::Side::Arm:
                        b->setStage(0);
                        break;
                    case MainMenuModel::PlayerSettings::Side::Core:
                        b->setStage(1);
                        break;
                }
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // color
            unsigned int width = 19;
            unsigned int height = 19;

            auto graphics = sceneContext.textureService->getGafEntry("anims/LOGOS.GAF", "32xlogos");
            auto newSprites = std::make_shared<SpriteSeries>();
            newSprites->sprites.reserve(graphics->sprites.size());

            std::transform(graphics->sprites.begin(), graphics->sprites.end(), std::back_inserter(newSprites->sprites), [width, height](const auto& sprite) {
                auto bounds = Rectangle2f::fromTopLeft(0.0f, 0.0f, width, height);
                return std::make_shared<Sprite>(bounds, sprite->texture, sprite->mesh);
            });

            auto b = uiFactory.createButton(214, rowStart, width, height, guiName, "logo", "");
            b->setName("PLAYER" + std::to_string(i) + "_color");

            auto sub = model.players[i].colorIndex.subscribe([b = b.get(), newSprites](const auto& index) {
                b->setNormalSprite(newSprites->sprites[index.value]);
                b->setPressedSprite(newSprites->sprites[index.value]);
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // ally
            unsigned int width = 38;
            unsigned int height = 20;

            auto graphics = sceneContext.textureService->getGuiTexture(guiName, "TEAMICONSx");
            if (!graphics)
            {
                graphics = sceneContext.textureService->getGuiTexture(guiName, "ally icons");
            }
            if (!graphics)
            {
                throw std::runtime_error("Failed to load TEAMICONSx");
            }

            auto b = uiFactory.createButton(241, rowStart, width, height, guiName, "team", "");
            b->setName("PLAYER" + std::to_string(i) + "_team");

            auto sub = model.players[i].teamIndex.subscribe([b = b.get(), &m = model, g = *graphics](auto index) {
                if (!index)
                {
                    b->setNormalSprite(g->sprites[10]);
                    b->setPressedSprite(g->sprites[10]);
                    return;
                }

                auto stage = (*index) * 2;
                if (!m.isTeamShared(*index))
                {
                    ++stage;
                }

                b->setNormalSprite(g->sprites[stage]);
                b->setPressedSprite(g->sprites[stage]);
            });
            b->addSubscription(std::move(sub));

            auto teamSub = model.teamChanges.subscribe([b = b.get(), &m = model, g = *graphics, i](auto index) {
                if (index == m.players[i].teamIndex.getValue())
                {
                    auto stage = index * 2;
                    if (m.isTeamShared(index))
                    {
                        b->setNormalSprite(g->sprites[stage]);
                        b->setPressedSprite(g->sprites[stage]);
                    }
                    else
                    {
                        b->setNormalSprite(g->sprites[stage + 1]);
                        b->setPressedSprite(g->sprites[stage + 1]);
                    }
                }
            });
            b->addSubscription(std::move(teamSub));

            panel.appendChild(std::move(b));
        }

        {
            // metal
            unsigned int width = 46;
            unsigned int height = 20;

            auto b = uiFactory.createButton(286, rowStart, width, height, guiName, "skirmmet", "");
            b->setName("PLAYER" + std::to_string(i) + "_metal");
            b->setTextAlign(UiStagedButton::TextAlign::Center);

            auto sub = model.players[i].metal.subscribe([b = b.get()](const auto& newMetal) {
                b->setLabel(formatResource(newMetal));
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // energy
            unsigned int width = 46;
            unsigned int height = 20;

            auto b = uiFactory.createButton(337, rowStart, width, height, guiName, "skirmmet", "");
            b->setName("PLAYER" + std::to_string(i) + "_energy");
            b->setTextAlign(UiStagedButton::TextAlign::Center);

            auto sub = model.players[i].energy.subscribe([b = b.get()](const auto& newEnergy) {
                b->setLabel(formatResource(newEnergy));
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }
    }

    bool MainMenuScene::hasMultiplayerSchema(const std::string& mapName)
    {
        auto otaRaw = sceneContext.vfs->readFile(std::string("maps/").append(mapName));
        if (!otaRaw)
        {
            throw std::runtime_error("Failed to read OTA file");
        }

        return parseTdfHasNetworkSchema(*otaRaw);
    }
}
