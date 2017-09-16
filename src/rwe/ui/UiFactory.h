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
#include <rwe/MainMenuModel.h>
#include <rwe/ui/UiScrollBar.h>

namespace rwe
{
    class MainMenuController;

    class UiFactory
    {
    private:
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;
        AbstractVirtualFileSystem* vfs;
        MainMenuModel* model;
        MainMenuController* controller;

    public:
        UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs, MainMenuModel* model, MainMenuController* controller);

        std::unique_ptr<UiPanel> panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries);

    private:
        std::unique_ptr<UiComponent> componentFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiButton> buttonFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiStagedButton> stagedButtonFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiLabel> labelFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiScrollBar> scrollBarFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiListBox> listBoxFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(const std::string& guiName, int width, int height);

        boost::optional<AudioService::SoundHandle> getButtonSound(const std::string& buttonName);

        boost::optional<AudioService::SoundHandle> deduceButtonSound(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultStagedButtonGraphics(const std::string& guiName, int stages);

        void attachDefaultEventHandlers(const std::string& guiName, UiPanel& panel);

        std::unique_ptr<UiComponent> surfaceFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        void attachPlayerSelectionComponents(const std::string& guiName, UiPanel& panel);

        void attachDetailedPlayerSelectionComponents(const std::string& guiName, UiPanel& panel, int i);
    };
}


#endif
