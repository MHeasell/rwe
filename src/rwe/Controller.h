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
        TdfBlock* allSoundTdf;
        AudioService* audioService;
        TextureService* textureService;

        UiFactory uiFactory;

        AudioService::LoopToken bgmHandle;
    public:

        Controller(AbstractVirtualFileSystem *vfs, SceneManager *sceneManager,
                   TdfBlock *allSoundTdf, AudioService *audioService, TextureService* textureService);

        void goToMainMenu();

        void goToSingleMenu();

        void startBGM();

        void start();

        void exit();

        void message(const std::string& topic, const std::string& message);
    };
}


#endif
