#include "Controller.h"

#include <rwe/gui.h>
#include <rwe/UiPanelScene.h>

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
        TextureService* textureService)
            : vfs(vfs),
              sceneManager(sceneManager),
              allSoundTdf(allSoundTdf),
              audioService(audioService),
              textureService(textureService),
              uiFactory(textureService, audioService, allSoundTdf, this),
              scene(std::make_shared<UiPanelScene>(audioService, allSoundTdf, 640, 480))
        {}

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
                openMapSelectionDialog();
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
}
