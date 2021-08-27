#pragma once

#include <rwe/AudioService.h>
#include <rwe/MainMenuModel.h>
#include <rwe/PathMapping.h>
#include <rwe/TextureService.h>
#include <rwe/io/gui/gui.h>
#include <rwe/ui/UiLabel.h>
#include <rwe/ui/UiListBox.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/ui/UiScrollBar.h>
#include <rwe/ui/UiStagedButton.h>
#include <string>
#include <vector>

namespace rwe
{
    class UiFactory
    {
    private:
        struct ButtonSprites
        {
            std::vector<std::shared_ptr<Sprite>> normal;
            std::shared_ptr<Sprite> pressed;
            std::shared_ptr<Sprite> disabled;
        };

    private:
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;
        AbstractVirtualFileSystem* vfs;
        const PathMapping* const pathMapping;

        int screenWidth;
        int screenHeight;

    public:
        UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs, const PathMapping* const pathMapping, int screenWidth, int screenHeight);

        std::unique_ptr<UiPanel> panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries);

        std::unique_ptr<UiPanel> panelFromGuiFile(const std::string& name);

        std::unique_ptr<UiPanel> panelFromGuiFile(const std::string& name, const std::vector<GuiEntry>& entries);

        std::unique_ptr<UiPanel> createPanel(int x, int y, int width, int height, const std::string& name);
        std::unique_ptr<UiPanel> createPanel(int x, int y, int width, int height, const std::string& name, const std::optional<std::string>& background);

        std::unique_ptr<UiStagedButton> createButton(int x, int y, int width, int height, const std::string& guiName, const std::string& name, const std::string& label);

        std::unique_ptr<UiStagedButton> createBasicButton(int x, int y, int width, int height, const std::string& guiName, const std::string& name, const std::string& label);

        std::unique_ptr<UiStagedButton> createStagedButton(int x, int y, int width, int height, const std::string& guiName, const std::string& name, const std::vector<std::string>& labels, int stages);

    private:
        std::unique_ptr<UiComponent> componentFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiStagedButton> buttonFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiStagedButton> stagedButtonFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiLabel> labelFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiScrollBar> scrollBarFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::unique_ptr<UiListBox> listBoxFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(const std::string& guiName, int width, int height);

        std::optional<AudioService::SoundHandle> getButtonSound(const std::string& buttonName);

        std::optional<AudioService::SoundHandle> deduceButtonSound(const std::string& guiName, const GuiEntry& entry);

        std::optional<AudioService::SoundHandle> deduceButtonSound(const std::string& guiName, const std::string& name, int width, int height);

        std::shared_ptr<SpriteSeries> getDefaultStagedButtonGraphics(const std::string& guiName, int stages);

        std::unique_ptr<UiComponent> surfaceFromGuiEntry(const std::string& guiName, const GuiEntry& entry);

        ButtonSprites getButtonGraphics(const std::string& guiName, const std::string& name, int width, int height);

        ButtonSprites getBasicButtonGraphics(const std::string& guiName, const std::string& name, int width, int height);

        ButtonSprites getStagedButtonGraphics(const std::string& guiName, const std::string& name, int stages);
    };
}
