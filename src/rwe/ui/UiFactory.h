#ifndef RWE_UIFACTORY_H
#define RWE_UIFACTORY_H

#include <rwe/AudioService.h>
#include <rwe/TextureService.h>
#include <rwe/gui.h>
#include <rwe/ui/UiButton.h>
#include <rwe/ui/UiListBox.h>
#include <rwe/ui/UiPanel.h>
#include <string>
#include <vector>
#include <rwe/ui/UiLabel.h>
#include <rwe/ui/UiStagedButton.h>
#include <rwe/SkirmishMenuModel.h>
#include <rwe/ui/UiScrollBar.h>

namespace rwe
{
    class Controller;

    class UiFactory
    {
    private:
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;
        AbstractVirtualFileSystem* vfs;
        SkirmishMenuModel* model;
        Controller* controller;

    public:
        UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs, SkirmishMenuModel* model, Controller* controller);

        UiPanel panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries);

    private:
        std::unique_ptr<UiButton> buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiStagedButton> stagedButtonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiLabel> labelFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiScrollBar> scrollBarFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiListBox> listBoxFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(const std::string& guiName, int width, int height);

        boost::optional<AudioService::SoundHandle> getButtonSound(const std::string& buttonName);

        boost::optional<AudioService::SoundHandle> deduceButtonSound(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultStagedButtonGraphics(const std::string& guiName, int stages);

        std::unique_ptr<UiComponent> createComponentFromGui(const std::string& guiName, const GuiEntry& entry);
    };
}


#endif
