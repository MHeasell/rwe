#ifndef RWE_UIFACTORY_H
#define RWE_UIFACTORY_H

#include <rwe/AudioService.h>
#include <rwe/TextureService.h>
#include <rwe/gui.h>
#include <rwe/ui/UiButton.h>
#include <rwe/ui/UiPanel.h>
#include <string>
#include <vector>

namespace rwe
{
    class UiFactory
    {
    private:
        TextureService* textureService;
        AudioService* audioService;
        TdfBlock* soundLookup;

    public:
        UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup);

        UiPanel panelFromGuiFile(const std::string& name, const std::vector<GuiEntry>& entries);

    private:
        UiButton buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(int width, int height);
    };
}


#endif
