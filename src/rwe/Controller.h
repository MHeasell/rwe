#ifndef RWE_MENUCONTEXT_H
#define RWE_MENUCONTEXT_H

#include <memory>
#include <rwe/AudioService.h>
#include <rwe/CursorService.h>
#include <rwe/SkirmishMenuModel.h>
#include <rwe/UiPanelScene.h>
#include <rwe/observable/Subscription.h>
#include <rwe/tdf/SimpleTdfAdapter.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>

#include "SceneManager.h"

namespace rwe
{
    class Controller
    {
    private:
        AbstractVirtualFileSystem* vfs;
        SceneManager* sceneManager;
        TdfBlock* allSoundTdf;
        AudioService* audioService;
        TextureService* textureService;
        CursorService* cursor;
        SkirmishMenuModel* model;

        UiFactory uiFactory;
        std::shared_ptr<UiPanelScene> scene;

        AudioService::LoopToken bgmHandle;

        std::vector<std::unique_ptr<Subscription>> subscriptions;

    public:
        Controller(
            AbstractVirtualFileSystem* vfs,
            SceneManager* sceneManager,
            TdfBlock* allSoundTdf,
            AudioService* audioService,
            TextureService* textureService,
            CursorService* cursor,
            SkirmishMenuModel* model);

        ~Controller();

        void goToMainMenu();

        void goToSingleMenu();

        void goToSkirmishMenu();

        void goToPreviousMenu();

        void openMapSelectionDialog();

        void start();

        void exit();

        void message(const std::string& topic, const std::string& message);

        void scrollMessage(const std::string& topic, unsigned int group, const std::string& name, const ScrollPositionMessage& m);

        void scrollUpMessage(const std::string& topic, unsigned int group, const std::string& name);

        void scrollDownMessage(const std::string& topic, unsigned int group, const std::string& name);

        void setCandidateSelectedMap(const std::string& mapName);

        void clearCandidateSelectedMap();

        void commitSelectedMap();

        void resetCandidateSelectedMap();

        void togglePlayer(int playerIndex);

        void incrementPlayerMetal(int playerIndex);

        void incrementPlayerEnergy(int playerIndex);

        void togglePlayerSide(int playerIndex);

        void cyclePlayerColor(int playerIndex);

        void cyclePlayerTeam(int playerIndex);
    };
}


#endif
