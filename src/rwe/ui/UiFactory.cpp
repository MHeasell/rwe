#include "UiFactory.h"
#include <rwe/ui/UiSurface.h>

#include <memory>
#include <rwe/MainMenuScene.h>
#include <rwe/rwe_string.h>
#include <rwe/ui/UiComponent.h>

namespace rwe
{
    UiFactory::UiFactory(TextureService* textureService, AudioService* audioService, TdfBlock* soundLookup, AbstractVirtualFileSystem* vfs, MainMenuModel* model)
        : textureService(textureService), audioService(audioService), soundLookup(soundLookup), vfs(vfs), model(model)
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

        if (name == "SKIRMISH")
        {
            attachPlayerSelectionComponents(name, *panel);
        }

        // set the default focused control
        if (panelEntry.defaultFocus)
        {
            const auto& focusName = *panelEntry.defaultFocus;
            panel->setFocusByName(focusName);
        }

        return panel;
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
        // hack for SINGLE.GUI buttons
        int width = entry.common.width;
        int height = entry.common.height;
        if (width == 118 && height == 18)
        {
            width = 120;
            height = 20;
        }


        auto graphics = textureService->getGuiTexture(guiName, entry.common.name);
        if (!graphics)
        {
            graphics = getDefaultButtonGraphics(guiName, width, height);
        }

        auto text = entry.text ? *(entry.text) : std::string("");

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto button = std::make_unique<UiButton>(
            entry.common.xpos,
            entry.common.ypos,
            width,
            height,
            *graphics,
            text,
            font);

        auto sound = deduceButtonSound(guiName, entry);

        if (sound)
        {
            button->onClick().subscribe([as = audioService, s = std::move(*sound)](const auto& /*param*/) {
                as->playSound(s);
            });
        }

        return button;
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
        auto graphics = textureService->getGuiTexture(guiName, entry.common.name);
        if (!graphics)
        {
            graphics = getDefaultStagedButtonGraphics(guiName, *entry.stages);
        }

        auto labels = entry.text ? utf8Split(*entry.text, '|') : std::vector<std::string>();

        auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");

        auto button = std::make_unique<UiStagedButton>(
            entry.common.xpos,
            entry.common.ypos,
            entry.common.width,
            entry.common.height,
            *graphics,
            labels,
            font);

        auto sound = deduceButtonSound(guiName, entry);

        if (sound)
        {
            button->onClick().subscribe([as = audioService, s = std::move(*sound)](const auto& /*param*/) {
                as->playSound(s);
            });
        }

        return button;
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
        auto sound = getButtonSound(entry.common.name);
        if (!sound && (entry.common.name == "PrevMenu" || entry.common.name == "PREVMENU"))
        {
            sound = getButtonSound("PREVIOUS");
        }
        if (!sound && entry.common.name == "Start")
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
        if (!sound && entry.common.width == 96 && entry.common.height == 20)
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

    void UiFactory::attachPlayerSelectionComponents(const std::string& guiName, UiPanel& panel)
    {
        unsigned int tableStart = 78;
        unsigned int rowHeight = 20;

        auto sound = getButtonSound(guiName);

        for (int i = 0; i < 10; ++i)
        {
            unsigned int rowStart = tableStart + (i * rowHeight);

            {
                // player name button
                unsigned int width = 112;
                unsigned int height = 20;

                auto graphics = textureService->getGuiTexture(guiName, "skirmname");
                if (!graphics)
                {
                    graphics = getDefaultButtonGraphics(guiName, width, height);
                }
                auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
                auto b = std::make_unique<UiButton>(45, rowStart, width, height, *graphics, "Player", font);
                b->setName("PLAYER" + std::to_string(i));
                if (sound)
                {
                    b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                        as->playSound(s);
                    });
                }

                auto sub = model->players[i].type.subscribe([b = b.get(), &panel, this, guiName, i](MainMenuModel::PlayerSettings::Type type) {
                    switch (type)
                    {
                        case MainMenuModel::PlayerSettings::Type::Open:
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            b->setLabel("Open");
                            break;
                        case MainMenuModel::PlayerSettings::Type::Human:
                            b->setLabel("Player");
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            attachDetailedPlayerSelectionComponents(guiName, panel, i);
                            break;
                        case MainMenuModel::PlayerSettings::Type::Computer:
                            b->setLabel("Computer");
                            panel.removeChildrenWithPrefix("PLAYER" + std::to_string(i) + "_");
                            attachDetailedPlayerSelectionComponents(guiName, panel, i);
                            break;
                    }
                });
                b->addSubscription(std::move(sub));

                panel.appendChild(std::move(b));
            }
        }
    }

    void UiFactory::attachDetailedPlayerSelectionComponents(const std::string& guiName, UiPanel& panel, int i)
    {
        unsigned int tableStart = 78;
        unsigned int rowHeight = 20;

        auto sound = getButtonSound(guiName);

        unsigned int rowStart = tableStart + (i * rowHeight);

        {
            // side button
            unsigned int width = 44;
            unsigned int height = 20;

            auto graphics = textureService->getGuiTexture(guiName, "SIDEx");
            if (!graphics)
            {
                graphics = getDefaultButtonGraphics(guiName, width, height);
            }
            auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
            auto b = std::make_unique<UiStagedButton>(163, rowStart, width, height, *graphics, std::vector<std::string>(2), font);
            b->setName("PLAYER" + std::to_string(i) + "_side");
            b->autoChangeStage = false;
            if (sound)
            {
                b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                    as->playSound(s);
                });
            }

            auto sub = model->players[i].side.subscribe([b = b.get()](MainMenuModel::PlayerSettings::Side side) {
                switch (side)
                {
                    case MainMenuModel::PlayerSettings::Side::Arm:
                        b->setStage(0);
                        break;
                    case MainMenuModel::PlayerSettings::Side::Core:
                        b->setStage(1);
                        break;
                }
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // color
            unsigned int width = 19;
            unsigned int height = 19;

            auto graphics = textureService->getGafEntry("anims/LOGOS.GAF", "32xlogos");

            auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
            auto b = std::make_unique<UiButton>(214, rowStart, width, height, graphics->sprites[0], graphics->sprites[0], std::string(), font);
            b->setName("PLAYER" + std::to_string(i) + "_color");
            if (sound)
            {
                b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                    as->playSound(s);
                });
            }

            auto sub = model->players[i].colorIndex.subscribe([b = b.get(), graphics](unsigned int index) {
                b->setNormalSprite(graphics->sprites[index]);
                b->setPressedSprite(graphics->sprites[index]);
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // ally
            unsigned int width = 38;
            unsigned int height = 20;

            auto graphics = textureService->getGuiTexture(guiName, "TEAMICONSx");
            if (!graphics)
            {
                graphics = getDefaultButtonGraphics(guiName, width, height);
            }

            auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
            auto b = std::make_unique<UiButton>(241, rowStart, width, height, (*graphics)->sprites[10], (*graphics)->sprites[10], std::string(), font);
            b->setName("PLAYER" + std::to_string(i) + "_team");
            if (sound)
            {
                b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                    as->playSound(s);
                });
            }

            auto sub = model->players[i].teamIndex.subscribe([b = b.get(), m = model, g = *graphics](auto index) {
                if (!index)
                {
                    b->setNormalSprite(g->sprites[10]);
                    b->setPressedSprite(g->sprites[10]);
                    return;
                }

                auto stage = (*index) * 2;
                if (!m->isTeamShared(*index))
                {
                    ++stage;
                }

                b->setNormalSprite(g->sprites[stage]);
                b->setPressedSprite(g->sprites[stage]);
            });
            b->addSubscription(std::move(sub));

            auto teamSub = model->teamChanges.subscribe([b = b.get(), m = model, g = *graphics, i](auto index) {
                if (index == m->players[i].teamIndex.getValue())
                {
                    auto stage = index * 2;
                    if (m->isTeamShared(index))
                    {
                        b->setNormalSprite(g->sprites[stage]);
                        b->setPressedSprite(g->sprites[stage]);
                    }
                    else
                    {
                        b->setNormalSprite(g->sprites[stage + 1]);
                        b->setPressedSprite(g->sprites[stage + 1]);
                    }
                }
            });
            b->addSubscription(std::move(teamSub));

            panel.appendChild(std::move(b));
        }

        {
            // metal
            unsigned int width = 46;
            unsigned int height = 20;

            auto graphics = textureService->getGuiTexture(guiName, "skirmmet");
            if (!graphics)
            {
                graphics = getDefaultButtonGraphics(guiName, width, height);
            }
            auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
            auto b = std::make_unique<UiButton>(286, rowStart, width, height, *graphics, "1000", font);
            b->setName("PLAYER" + std::to_string(i) + "_metal");
            if (sound)
            {
                b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                    as->playSound(s);
                });
            }

            auto sub = model->players[i].metal.subscribe([b = b.get()](int newMetal) {
                b->setLabel(std::to_string(newMetal));
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }

        {
            // energy
            unsigned int width = 46;
            unsigned int height = 20;

            auto graphics = textureService->getGuiTexture(guiName, "skirmmet");
            if (!graphics)
            {
                graphics = getDefaultButtonGraphics(guiName, width, height);
            }
            auto font = textureService->getGafEntry("anims/hattfont12.gaf", "Haettenschweiler (120)");
            auto b = std::make_unique<UiButton>(337, rowStart, width, height, *graphics, "1000", font);
            b->setName("PLAYER" + std::to_string(i) + "_energy");
            if (sound)
            {
                b->onClick().subscribe([as = audioService, s = *sound](const auto& /*param*/) {
                    as->playSound(s);
                });
            }

            auto sub = model->players[i].energy.subscribe([b = b.get()](int newEnergy) {
                b->setLabel(std::to_string(newEnergy));
            });
            b->addSubscription(std::move(sub));

            panel.appendChild(std::move(b));
        }
    }
}
