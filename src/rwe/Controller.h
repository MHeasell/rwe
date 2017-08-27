#ifndef RWE_MENUCONTEXT_H
#define RWE_MENUCONTEXT_H


#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <rwe/ui/UiFactory.h>
#include <rwe/AudioService.h>
#include <rwe/tdf/SimpleTdfAdapter.h>

#include "SceneManager.h"

namespace rwe
{
    class Controller
    {
    private:
        AbstractVirtualFileSystem* vfs;
        SceneManager* sceneManager;
        UiFactory* uiFactory;
        TdfBlock* allSoundTdf;
        AudioService* audioService;

        AudioService::LoopToken bgmHandle;
    public:

        Controller(AbstractVirtualFileSystem *vfs, SceneManager *sceneManager, UiFactory *uiFactory,
                   TdfBlock *allSoundTdf, AudioService *audioService);

        void goToMainMenu();

        void startBGM();

        void start();
    };
}


#endif
