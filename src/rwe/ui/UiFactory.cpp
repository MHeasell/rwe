#include "UiFactory.h"

#include <memory>
#include <rwe/ui/UiComponent.h>
#include <rwe/Controller.h>

namespace rwe
{
    UiPanel UiFactory::panelFromGuiFile(const std::string& name, const std::vector<GuiEntry>& entries)
    {
        // first entry sets up the panel
        assert(entries.size() > 0);
        auto panelEntry = entries[0];

        auto texture = textureService->getBitmap("FrontendX");
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
                case GuiElementType::Button:
                    std::unique_ptr<UiComponent> btn(new UiButton(buttonFromGuiFile(name, entry)));
                    panel.appendChild(std::move(btn));
                    break;
            }
        }

        return panel;
    }

    UiButton UiFactory::buttonFromGuiFile(const std::string& guiName, const GuiEntry& entry)
    {

        auto graphics = textureService->getGuiTexture(guiName, entry.common.name);
        if (!graphics)
        {
            graphics = getDefaultButtonGraphics(entry.common.width, entry.common.height);
        }

        auto text = entry.text ? *(entry.text) : std::string("");

        UiButton button(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            *graphics,
            text
        );

        auto soundBlock = soundLookup->findBlock("BIGBUTTON");
        if (soundBlock)
        {
            auto soundName = soundBlock->findValue("sound");
            if (soundName)
            {
                auto sound = audioService->loadSound(*soundName);
                if (sound)
                {
                    button.onClick([as = audioService, s = std::move(*sound)](MouseButtonEvent /*event*/){
                        as->playSound(s);
                    });
                }
            }
        }

        button.onClick([c = controller, guiName, name = entry.common.name](MouseButtonEvent /*event*/){
            c->message(guiName, name);
        });

        return button;
    }

    UiFactory::UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, Controller* controller)
        : textureService(textureService), audioService(audioService), soundLookup(soundLookup), controller(controller)
    {}

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultButtonGraphics(int width, int height)
    {
        if (width == 96 && height == 20)
        {
            auto texture = textureService->getBitmap("BUTTONS2");
            Sprite normal(
                Rectangle2f::fromTopLeft(0.0f, 0.0f, 96.0f, 20.0f),
                texture,
                Rectangle2f::fromTopLeft(9.0f / 640.0f, 221.0f / 480.0f, 96.0f / 640.0f, 20.0f / 480.0f)
            );
            Sprite pressed(
                Rectangle2f::fromTopLeft(0.0f, 0.0f, 96.0f, 20.0f),
                texture,
                Rectangle2f::fromTopLeft(9.0f / 640.0f, 251.0f / 480.0f, 96.0f / 640.0f, 20.0f / 480.0f)
            );
            auto series = std::make_shared<SpriteSeries>();
            series->sprites.push_back(normal);
            series->sprites.push_back(pressed);
            return series;
        }
        else
        {
            auto texture = textureService->getDefaultTexture();
            Sprite sprite(Rectangle2f::fromTopLeft(0.0f, 0.0f, width, height), texture);
            auto series = std::make_shared<SpriteSeries>();
            series->sprites.push_back(sprite);
            series->sprites.push_back(sprite);
            return series;
        }
    }
}
