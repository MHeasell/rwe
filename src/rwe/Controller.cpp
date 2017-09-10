#include "Controller.h"

#include <rwe/UiPanelScene.h>
#include <rwe/gui.h>
#include <rwe/ota.h>
#include <rwe/tdf.h>

#include <stdexcept>

namespace rwe
{
    void Controller::goToMainMenu()
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

    void Controller::start()
    {
        goToMainMenu();
        sceneManager->setNextScene(scene);
        sceneManager->execute();
    }

    Controller::Controller(
        AbstractVirtualFileSystem* vfs,
        SceneManager* sceneManager,
        TdfBlock* allSoundTdf,
        AudioService* audioService,
        TextureService* textureService,
        CursorService* cursor,
        SkirmishMenuModel* model)
        : vfs(vfs),
          sceneManager(sceneManager),
          allSoundTdf(allSoundTdf),
          audioService(audioService),
          textureService(textureService),
          cursor(cursor),
          model(model),
          uiFactory(textureService, audioService, allSoundTdf, vfs, model, this),
          scene(std::make_shared<UiPanelScene>(audioService, allSoundTdf, cursor, 640, 480))
    {
    }

    void Controller::exit()
    {
        sceneManager->requestExit();
    }

    void Controller::message(const std::string& topic, const std::string& message)
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

    void Controller::goToSingleMenu()
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

    void Controller::goToSkirmishMenu()
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

    void Controller::goToPreviousMenu()
    {
        if (!scene->hasPreviousMenu())
        {
            sceneManager->requestExit();
            return;
        }

        scene->goToPreviousMenu();
    }

    void Controller::openMapSelectionDialog()
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

    void Controller::setCandidateSelectedMap(const std::string& mapName)
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

        SkirmishMenuModel::SelectedMapInfo info(
            mapName,
            ota.missionDescription,
            sizeInfo,
            minimap
        );

        model->candidateSelectedMap.next(std::move(info));
    }

    void Controller::resetCandidateSelectedMap()
    {
        model->candidateSelectedMap.next(model->selectedMap.getValue());
    }

    void Controller::commitSelectedMap()
    {
        model->selectedMap.next(model->candidateSelectedMap.getValue());
    }

    void Controller::clearCandidateSelectedMap()
    {
        model->candidateSelectedMap.next(boost::none);
    }

    void Controller::scrollMessage(
        const std::string& topic,
        unsigned int group,
        const std::string& name,
        const ScrollPositionMessage& message)
    {
        GroupMessage gm(topic, group, name, message);
        model->groupMessages.next(gm);
    }

    Controller::~Controller()
    {
        for (auto& sub : subscriptions)
        {
            sub->unsubscribe();
        }
    }

    void Controller::scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollUpMessage m;
        model->groupMessages.next(GroupMessage(topic, group, name, m));
    }

    void Controller::scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name)
    {
        ScrollDownMessage m;
        model->groupMessages.next(GroupMessage(topic, group, name, m));
    }
}
