#include "MainMenuController.h"

#include <rwe/MainMenuScene.h>
#include <rwe/gui.h>
#include <rwe/ota.h>
#include <rwe/tdf.h>

#include <stdexcept>

namespace rwe
{
    void MainMenuController::goToMainMenu()
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
        scene->goToMenu(std::move(panel));
    }

    void MainMenuController::start()
    {
        goToMainMenu();
        sceneManager->setNextScene(scene);
        sceneManager->execute();
    }

    MainMenuController::MainMenuController(
        AbstractVirtualFileSystem* vfs,
        SceneManager* sceneManager,
        TdfBlock* allSoundTdf,
        AudioService* audioService,
        TextureService* textureService,
        CursorService* cursor,
        MainMenuModel* model)
        : vfs(vfs),
          sceneManager(sceneManager),
          allSoundTdf(allSoundTdf),
          audioService(audioService),
          textureService(textureService),
          cursor(cursor),
          model(model),
          uiFactory(textureService, audioService, allSoundTdf, vfs, model, this),
          scene(std::make_shared<MainMenuScene>(audioService, allSoundTdf, cursor, 640, 480))
    {
    }

    void MainMenuController::exit()
    {
        sceneManager->requestExit();
    }

    void MainMenuController::message(const std::string& topic, const std::string& message)
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

    void MainMenuController::goToSingleMenu()
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
        scene->goToMenu(std::move(panel));
    }

    void MainMenuController::goToSkirmishMenu()
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
        scene->goToMenu(std::move(panel));
    }

    void MainMenuController::goToPreviousMenu()
    {
        if (!scene->hasPreviousMenu())
        {
            sceneManager->requestExit();
            return;
        }

        scene->goToPreviousMenu();
    }

    void MainMenuController::openMapSelectionDialog()
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
        scene->openDialog(std::move(panel));
    }

    void MainMenuController::setCandidateSelectedMap(const std::string& mapName)
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

        model->candidateSelectedMap.next(std::move(info));
    }

    void MainMenuController::resetCandidateSelectedMap()
    {
        model->candidateSelectedMap.next(model->selectedMap.getValue());
    }

    void MainMenuController::commitSelectedMap()
    {
        model->selectedMap.next(model->candidateSelectedMap.getValue());
    }

    void MainMenuController::clearCandidateSelectedMap()
    {
        model->candidateSelectedMap.next(boost::none);
    }

    void MainMenuController::scrollMessage(
        const std::string& topic,
        unsigned int group,
        const std::string& name,
        const ScrollPositionMessage& message)
    {
        GroupMessage gm(topic, group, name, message);
        model->groupMessages.next(gm);
    }

    MainMenuController::~MainMenuController()
    {
        for (auto& sub : subscriptions)
        {
            sub->unsubscribe();
        }
    }

    void MainMenuController::scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollUpMessage m;
        model->groupMessages.next(GroupMessage(topic, group, name, m));
    }

    void MainMenuController::scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollDownMessage m;
        model->groupMessages.next(GroupMessage(topic, group, name, m));
    }

    void MainMenuController::incrementPlayerMetal(int playerIndex)
    {
        auto& player = model->players[playerIndex];

        if (player.metal.getValue() == 200)
        {
            player.metal.next(500);
        }
        else if (player.metal.getValue() < 10000)
        {
            player.metal.next(player.metal.getValue() + 500);
        }
    }

    void MainMenuController::decrementPlayerMetal(int playerIndex)
    {
        auto& player = model->players[playerIndex];

        if (player.metal.getValue() > 500)
        {
            player.metal.next(player.metal.getValue() - 500);
        }
        else if (player.metal.getValue() == 500)
        {
            player.metal.next(200);
        }
    }

    void MainMenuController::incrementPlayerEnergy(int playerIndex)
    {
        auto& player = model->players[playerIndex];

        if (player.energy.getValue() == 200)
        {
            player.energy.next(500);
        }
        else if (player.energy.getValue() < 10000)
        {
            player.energy.next(player.energy.getValue() + 500);
        }
    }

    void MainMenuController::decrementPlayerEnergy(int playerIndex)
    {
        auto& player = model->players[playerIndex];

        if (player.energy.getValue() > 500)
        {
            player.energy.next(player.energy.getValue() - 500);
        }
        else if (player.energy.getValue() == 500)
        {
            player.energy.next(200);
        }
    }

    void MainMenuController::togglePlayer(int playerIndex)
    {
        auto& player = model->players[playerIndex];

        switch (player.type.getValue())
        {
            case MainMenuModel::PlayerSettings::Type::Open:
            {
                auto color = player.colorIndex.getValue();
                if (model->isColorInUse(color))
                {
                    auto newColor = model->getFirstFreeColor();
                    if (!newColor)
                    {
                        throw std::logic_error("No free colors for the player");
                    }
                    color = *newColor;
                }

                bool humanAllowed = std::find_if(model->players.begin(), model->players.end(), [](const auto& p) {
                    return p.type.getValue() == MainMenuModel::PlayerSettings::Type::Human;
                }) == model->players.end();
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

    void MainMenuController::togglePlayerSide(int playerIndex)
    {
        auto& player = model->players[playerIndex];
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

    void MainMenuController::cyclePlayerColor(int playerIndex)
    {
        auto& player = model->players[playerIndex];
        auto currentColor = player.colorIndex.getValue();
        for (int i = 1; i < 10; ++i)
        {
            auto newColor = (currentColor + i) % 10;
            if (!model->isColorInUse(newColor))
            {
                player.colorIndex.next(newColor);
                return;
            }
        }
    }

    void MainMenuController::reverseCyclePlayerColor(int playerIndex)
    {
        auto& player = model->players[playerIndex];
        auto currentColor = player.colorIndex.getValue();
        for (int i = 9; i >= 1; --i)
        {
            auto newColor = (currentColor + i) % 10;
            if (!model->isColorInUse(newColor))
            {
                player.colorIndex.next(newColor);
                return;
            }
        }
    }

    void MainMenuController::cyclePlayerTeam(int playerIndex)
    {
        auto& player = model->players[playerIndex];
        if (!player.teamIndex.getValue())
        {
            player.teamIndex.next(0);
            model->teamChanges.next(0);
        }
        else
        {
            auto val = *(player.teamIndex.getValue());
            if (val == 4)
            {
                player.teamIndex.next(boost::none);
                model->teamChanges.next(val);
            }
            else
            {
                player.teamIndex.next(val + 1);
                model->teamChanges.next(val);
                model->teamChanges.next(val + 1);
            }
        }
    }

    void MainMenuController::startGame()
    {

    }
}
