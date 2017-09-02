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

namespace rwe
{
    class Controller;

    class UiFactory
    {
    private:
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;
        Controller* controller;

    public:
        UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, Controller* controller);

        UiPanel panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries);

    private:
        UiButton buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        UiStagedButton stagedButtonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        UiLabel labelFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        UiListBox listBoxFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(const std::string& guiName, int width, int height);

        boost::optional<AudioService::SoundHandle> getButtonSound(const std::string& buttonName);

        std::shared_ptr<SpriteSeries> getDefaultStagedButtonGraphics(const std::string& guiName, int stages);
    };
}


#endif
