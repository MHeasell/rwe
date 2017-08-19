#include "UiFactory.h"

namespace rwe
{
    UiPanel UiFactory::panelFromGuiFile(const std::string& name, const std::vector<GuiEntry>& entries)
    {
        // first entry sets up the panel
        assert(entries.size() > 0);
        auto panelEntry = entries[0];

        auto texture = textureService->getBitmap("TITLSCRN");
        UiPanel panel(
            panelEntry.common.xpos,
            panelEntry.common.ypos,
            panelEntry.common.width,
            panelEntry.common.height,
            texture);

        // load panel components
        for (std::size_t i = 1; i < entries.size(); ++i)
        {
            auto& entry = entries[i];

            switch (entry.common.id)
            {
            }
        }

        return panel;
    }

    UiButton UiFactory::buttonFromGuiFile(const GuiEntry& entry)
    {

        auto graphics = getDefaultButtonGraphics(entry.common.width, entry.common.height);

        auto text = entry.text ? *(entry.text) : std::string("");

        UiButton button(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            graphics,
            text
        );

        return button;
    }

    UiFactory::UiFactory(TextureService* textureService) : textureService(textureService)
    {}

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultButtonGraphics(int width, int height)
    {
        return textureService->getDefaultSpriteSeries();
    }
}
