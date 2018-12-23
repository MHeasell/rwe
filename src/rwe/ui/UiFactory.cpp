#include "UiFactory.h"
#include <rwe/ui/UiSurface.h>

#include <memory>
#include <rwe/MainMenuScene.h>
#include <rwe/rwe_string.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    UiFactory::UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs)
        : textureService(textureService), audioService(audioService), soundLookup(soundLookup), vfs(vfs)
    {
    }

    std::unique_ptr<UiPanel> UiFactory::panelFromGuiFile(const std::string& name, const std::string& background, const std::vector<GuiEntry>& entries)
    {
        // first entry sets up the panel
        assert(entries.size() > 0);
        auto panelEntry = entries[0];

        auto texture = textureService->getBitmapRegion(
            background,
            0,
            0,
            panelEntry.common.width,
            panelEntry.common.height);


        // Adjust x and y pos such that the bottom and right edges of the panel
        // do not go over the edge of the screen.
        auto rightBound = panelEntry.common.xpos + panelEntry.common.width;
        int xPos = panelEntry.common.xpos;
        if (rightBound > 640)
        {
            xPos -= rightBound - 640;
        }

        auto bottomBound = panelEntry.common.ypos + panelEntry.common.height;
        int yPos = panelEntry.common.ypos;
        if (bottomBound > 480)
        {
            yPos -= bottomBound - 480;
        }

        auto panel = std::make_unique<UiPanel>(
            xPos,
            yPos,
            panelEntry.common.width,
            panelEntry.common.height,
            texture);
        panel->setName(name);

        // load panel components
        for (std::size_t i = 1; i < entries.size(); ++i)
        {
            auto& entry = entries[i];

            auto elem = componentFromGuiEntry(name, entry);

            elem->setName(entry.common.name);
            elem->setGroup(entry.common.assoc);

            panel->appendChild(std::move(elem));
        }

        // set the default focused control
        if (panelEntry.defaultFocus)
        {
            const auto& focusName = *panelEntry.defaultFocus;
            panel->setFocusByName(focusName);
        }

        return panel;
    }

    std::unique_ptr<UiButton>
    UiFactory::createButton(int x, int y, int width, int height, const std::string& guiName, const std::string& name, const std::string& label)
    {
        // hack for SINGLE.GUI buttons
        if (width == 118 && height == 18)
        {
            width = 120;
            height = 20;
        }


        auto graphics = textureService->getGuiTexture(guiName, name);
        if (!graphics)
        {
            graphics = getDefaultButtonGraphics(guiName, width, height);
        }

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto button = std::make_unique<UiButton>(x, y, width, height, *graphics, label, font);

        auto sound = deduceButtonSound(guiName, name, width, height);

        if (sound)
        {
            button->onClick().subscribe([as = audioService, s = std::move(*sound)](const auto& /*param*/) {
                as->playSound(s);
            });
        }

        return button;
    }

    std::unique_ptr<UiStagedButton>
    UiFactory::createStagedButton(int x, int y, int width, int height, const std::string& guiName, const std::string& name, const std::vector<std::string>& labels)
    {
        auto graphics = textureService->getGuiTexture(guiName, name);
        if (!graphics)
        {
            graphics = getDefaultStagedButtonGraphics(guiName, labels.size());
        }

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto button = std::make_unique<UiStagedButton>(x, y, width, height, *graphics, labels, font);

        auto sound = deduceButtonSound(guiName, name, width, height);

        if (sound)
        {
            button->onClick().subscribe([as = audioService, s = std::move(*sound)](const auto& /*param*/) {
                as->playSound(s);
            });
        }

        return button;
    }

    std::unique_ptr<UiComponent> UiFactory::componentFromGuiEntry(const std::string& guiName, const GuiEntry& entry)
    {
        switch (entry.common.id)
        {
            case GuiElementType::Button:
            {
                auto stages = entry.stages.value_or(0);
                if (stages > 1)
                {
                    return stagedButtonFromGuiEntry(guiName, entry);
                }
                else
                {
                    return buttonFromGuiEntry(guiName, entry);
                }
            }
            case GuiElementType::ListBox:
                return listBoxFromGuiEntry(guiName, entry);
            case GuiElementType::Label:
                return labelFromGuiEntry(guiName, entry);
            case GuiElementType::ScrollBar:
                return scrollBarFromGuiEntry(guiName, entry);
            case GuiElementType::Surface:
                return surfaceFromGuiEntry(guiName, entry);
            default:
                return std::make_unique<UiComponent>(0, 0, 1, 1);
        }
    }

    std::unique_ptr<UiButton> UiFactory::buttonFromGuiEntry(const std::string& guiName, const GuiEntry& entry)
    {
        auto text = entry.text ? *(entry.text) : std::string("");
        return createButton(
                entry.common.xpos,
                entry.common.ypos,
                entry.common.width,
                entry.common.height,
                guiName,
                entry.common.name,
                text);
    }

    std::unique_ptr<UiLabel> UiFactory::labelFromGuiEntry(const std::string& guiName, const GuiEntry& entry)
    {
        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto label = std::make_unique<UiLabel>(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            entry.text.value_or(""),
            font);

        return label;
    }

    std::unique_ptr<UiStagedButton> UiFactory::stagedButtonFromGuiEntry(
        const std::string& guiName,
        const GuiEntry& entry)
    {
        auto labels = entry.text ? utf8Split(*entry.text, '|') : std::vector<std::string>();
        return createStagedButton(
                entry.common.xpos,
                entry.common.ypos,
                entry.common.width,
                entry.common.height,
                guiName,
                entry.common.name,
                labels);
    }

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultStagedButtonGraphics(const std::string& guiName, int stages)
    {
        assert(stages >= 2 && stages <= 4);
        std::string entryName("stagebuttn");
        entryName.append(std::to_string(stages));

        auto sprites = textureService->getGuiTexture(guiName, entryName);
        if (sprites)
        {
            return *sprites;
        }

        // default behaviour
        const auto& sprite = textureService->getDefaultSprite();
        auto series = std::make_shared<SpriteSeries>();
        series->sprites.push_back(sprite);
        series->sprites.push_back(sprite);
        return series;
    }

    std::unique_ptr<UiListBox> UiFactory::listBoxFromGuiEntry(const std::string& /*guiName*/, const GuiEntry& entry)
    {
        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto listBox = std::make_unique<UiListBox>(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            font);

        return listBox;
    }

    std::optional<AudioService::SoundHandle> UiFactory::deduceButtonSound(const std::string& guiName, const GuiEntry& entry)
    {
        return deduceButtonSound(guiName, entry.common.name, entry.common.width, entry.common.height);
    }

    std::optional<AudioService::SoundHandle> UiFactory::deduceButtonSound(const std::string& guiName, const std::string& name, int width, int height)
    {
        auto sound = getButtonSound(name);
        if (!sound && (name == "PrevMenu" || name == "PREVMENU"))
        {
            sound = getButtonSound("PREVIOUS");
        }
        if (!sound && name == "Start")
        {
            sound = getButtonSound("BIGBUTTON");
        }
        if (!sound)
        {
            sound = getButtonSound(guiName);
        }
        if (!sound && guiName == "SELMAP")
        {
            sound = getButtonSound("SMALLBUTTON");
        }
        if (!sound && width == 96 && height == 20)
        {
            sound = getButtonSound("BIGBUTTON");
        }

        return sound;
    }

    std::unique_ptr<UiScrollBar> UiFactory::scrollBarFromGuiEntry(const std::string& guiName, const GuiEntry& entry)
    {
        auto sprites = textureService->getGuiTexture(guiName, "SLIDERS");
        if (!sprites)
        {
            throw std::runtime_error("Missing SLIDERS gaf entry");
        }

        auto scrollBar = std::make_unique<UiScrollBar>(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            *sprites);

        return scrollBar;
    }

    std::shared_ptr<SpriteSeries> UiFactory::getDefaultButtonGraphics(const std::string& guiName, int width, int height)
    {
        auto sprites = textureService->getGuiTexture(guiName, "BUTTONS0");
        if (sprites)
        {
            auto it = std::find_if(
                (*sprites)->sprites.begin(),
                (*sprites)->sprites.end(),
                [width, height](const std::shared_ptr<Sprite>& s) {
                    return s->bounds.width() == width && s->bounds.height() == height;
                });

            if (it != (*sprites)->sprites.end())
            {
                auto spritesView = std::make_shared<SpriteSeries>();
                spritesView->sprites.push_back(*(it++));
                assert(it != (*sprites)->sprites.end());
                spritesView->sprites.push_back(*(it++));
                return spritesView;
            }
        }

        // default behaviour
        const auto& sprite = textureService->getDefaultSprite();
        auto series = std::make_shared<SpriteSeries>();
        series->sprites.push_back(sprite);
        series->sprites.push_back(sprite);
        return series;
    }

    std::optional<AudioService::SoundHandle> UiFactory::getButtonSound(const std::string& buttonName)
    {
        auto soundBlock = soundLookup->findBlock(buttonName);
        if (!soundBlock)
        {
            return std::nullopt;
        }

        auto soundName = soundBlock->get().findValue("sound");
        if (!soundName)
        {
            return std::nullopt;
        }

        return audioService->loadSound(*soundName);
    }

    std::unique_ptr<UiComponent> UiFactory::surfaceFromGuiEntry(const std::string& guiName, const GuiEntry& entry)
    {
        auto surface = std::make_unique<UiSurface>(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height);

        return surface;
    }
}
