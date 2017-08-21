#ifndef RWE_UIFACTORY_H
#define RWE_UIFACTORY_H

#include <rwe/TextureService.h>
#include <rwe/gui.h>
#include <rwe/ui/UiPanel.h>
#include <rwe/ui/UiButton.h>
#include <vector>
#include <string>

namespace rwe
{
    class UiFactory
    {
    private:
        TextureService* textureService;

    public:
        explicit UiFactory(TextureService* textureService);

        UiPanel panelFromGuiFile(const std::string& name, const std::vector<GuiEntry>& entries);

    private:
        UiButton buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry);

        std::shared_ptr<SpriteSeries> getDefaultButtonGraphics(int width, int height);
    };
}


#endif
