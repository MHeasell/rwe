#include "gui.h"

#include <algorithm>
#include <rwe/optional_util.h>
#include <rwe/tdf.h>
#include <rwe/tdf/TdfBlock.h>
#include <stdexcept>

namespace rwe
{
    GuiParseException::GuiParseException(const std::string& message) : runtime_error(message)
    {
    }

    GuiParseException::GuiParseException(const char* message) : runtime_error(message)
    {
    }

    std::optional<GuiElementType> toGuiElementType(int value)
    {
        switch (value)
        {
            case static_cast<int>(GuiElementType::Panel):
            case static_cast<int>(GuiElementType::Button):
            case static_cast<int>(GuiElementType::ListBox):
            case static_cast<int>(GuiElementType::TextBox):
            case static_cast<int>(GuiElementType::ScrollBar):
            case static_cast<int>(GuiElementType::Label):
            case static_cast<int>(GuiElementType::Surface):
            case static_cast<int>(GuiElementType::Font):
            case static_cast<int>(GuiElementType::PictureBox):
                return static_cast<GuiElementType>(value);
            default:
                return std::nullopt;
        }
    }

    std::optional<GuiElementType> extractGuiElementType(const TdfBlock& block, const std::string& key)
    {
        auto value = block.extractInt(key);
        if (!value)
        {
            return std::nullopt;
        }

        return toGuiElementType(*value);
    }

    GuiElementType expectGuiElementType(const TdfBlock& block, const std::string& key)
    {
        auto v = extractGuiElementType(block, key);
        if (!v)
        {
            throw GuiParseException("Failed to read GUI element type from key: " + key);
        }

        return *v;
    }

    std::optional<GuiEntry> parseGuiEntry(const TdfBlock& e)
    {
        GuiEntry g;

        auto commonOption = e.findBlock("COMMON");
        if (!commonOption)
        {
            throw GuiParseException("Block is missing common section");
        }

        auto common = &commonOption->get();

        g.common.id = expectGuiElementType(*common, "id");
        g.common.assoc = common->extractInt("assoc").value_or(0);
        g.common.name = common->expectString("name");
        g.common.xpos = common->extractInt("xpos").value_or(0);
        g.common.ypos = common->extractInt("ypos").value_or(0);
        g.common.width = common->extractInt("width").value_or(0);
        g.common.height = common->extractInt("height").value_or(0);

        // "attribs" and "colorf" should be present in all gadgets,
        // but ARMGEN.GUI contains a gadget whose "attribs" property
        // is missing a semicolon.
        // This causes the parser to consume the next property, "colorf",
        // as the value of "attribs", making both invalid.
        // We explicitly provide defaults here to get around that issue.
        // Probably we should provide default values
        // for all the keys here, but I don't want to go through the effort
        // of deciding on reasonable defaults.
        g.common.attribs = common->extractInt("attribs").value_or(0);
        g.common.colorf = common->extractInt("colorf").value_or(0);

        g.common.colorb = common->extractInt("colorb").value_or(0);
        g.common.textureNumber = common->extractInt("texturenumber").value_or(0);
        g.common.fontNumber = common->extractInt("fontnumber").value_or(0);
        g.common.active = common->extractBool("active").value_or(false);
        g.common.commonAttribs = common->extractInt("commonattribs").value_or(0);
        g.common.help = refToCopy(common->findValue("help"));

        g.panel = refToCopy(e.findValue("panel"));
        g.crDefault = refToCopy(e.findValue("crdefault"));
        g.escdefault = refToCopy(e.findValue("escdefault"));
        g.defaultFocus = refToCopy(e.findValue("defaultfocus"));

        g.totalGadgets = e.extractInt("totalgadgets");

        auto versionOption = e.findBlock("VERSION");
        if (versionOption)
        {
            auto version = &versionOption->get();

            auto major = version->expectInt("major");
            auto minor = version->expectInt("minor");
            auto revision = version->expectInt("revision");
            g.version = GuiVersion(major, minor, revision);
        }

        g.status = e.extractInt("status");
        g.text = refToCopy(e.findValue("text"));
        g.quickKey = e.extractInt("quickkey");
        g.grayedOut = e.extractBool("grayedout");
        g.stages = e.extractInt("stages");

        return g;
    }

    std::optional<std::vector<GuiEntry>> parseGui(const TdfBlock& tdf)
    {
        std::vector<GuiEntry> entries;

        int i = 0;
        auto block = tdf.findBlock("GADGET" + std::to_string(i));
        while (block)
        {
            auto guiEntry = parseGuiEntry(*block);
            if (!guiEntry)
            {
                return std::nullopt;
            }

            entries.push_back(*guiEntry);

            i += 1;
            block = tdf.findBlock("GADGET" + std::to_string(i));
        }

        return entries;
    }

    std::optional<std::vector<GuiEntry>> parseGuiFromBytes(const std::vector<char>& bytes)
    {
        return parseGui(parseTdfFromBytes(bytes));
    }

    bool GuiEntry::operator==(const GuiEntry& rhs) const
    {
        return common == rhs.common
            && panel == rhs.panel
            && crDefault == rhs.crDefault
            && escdefault == rhs.escdefault
            && defaultFocus == rhs.defaultFocus
            && totalGadgets == rhs.totalGadgets
            && version == rhs.version
            && status == rhs.status
            && text == rhs.text
            && quickKey == rhs.quickKey
            && grayedOut == rhs.grayedOut
            && stages == rhs.stages;
    }

    bool GuiEntry::operator!=(const GuiEntry& rhs) const
    {
        return !(rhs == *this);
    }
}
