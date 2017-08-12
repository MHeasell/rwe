#include "gui.h"

#include <algorithm>
#include <stdexcept>

namespace rwe
{
    GuiParseException::GuiParseException(const std::string& message) : runtime_error(message)
    {
    }

    GuiParseException::GuiParseException(const char* message) : runtime_error(message)
    {
    }

    boost::optional<const std::string&> extractString(const TdfBlock& block, const std::string& key)
    {
        // find the key in the block
        auto pos = std::find_if(block.begin(), block.end(), [key](const TdfBlockEntry& e) { return e.name == key; });
        if (pos == block.end())
        {
            return boost::none;
        }

        // make sure the key contains a primitive (not a block) and extract it
        auto& valuePointer = pos->value;
        return boost::get<std::string>(*valuePointer);
    }

    boost::optional<const TdfBlock&> extractBlock(const TdfBlock& block, const std::string& key)
    {
        // find the key in the block
        auto pos = std::find_if(block.begin(), block.end(), [key](const TdfBlockEntry& e) { return e.name == key; });
        if (pos == block.end())
        {
            return boost::none;
        }

        // make sure the key contains a block and extract it
        auto& valuePointer = pos->value;
        return boost::get<TdfBlock>(*valuePointer);
    }

    const std::string& expectString(const TdfBlock& block, const std::string& key)
    {
        auto v = extractString(block, key);
        if (!v)
        {
            throw GuiParseException("Failed to read string from key: " + key);
        }

        return *v;
    }

    boost::optional<int> extractInt(const TdfBlock& block, const std::string& key)
    {
        auto value = extractString(block, key);
        if (!value)
        {
            return boost::none;
        }

        // convert the value to an integer
        try
        {
            return std::stoi(*value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    int expectInt(const TdfBlock& block, const std::string& key)
    {
        auto v = extractInt(block, key);
        if (!v)
        {
            throw GuiParseException("Failed to read int from key: " + key);
        }

        return *v;
    }

    boost::optional<bool> extractBool(const TdfBlock& block, const std::string& key)
    {
        auto value = extractString(block, key);
        if (!value)
        {
            return boost::none;
        }

        // convert the value to an integer
        int i;
        try
        {
            i = std::stoi(*value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }

        return i != 0;
    }

    bool expectBool(const TdfBlock& block, const std::string& key)
    {
        auto v = extractBool(block, key);
        if (!v)
        {
            throw GuiParseException("Failed to read bool from key: " + key);
        }

        return *v;
    }

    boost::optional<GuiElementType> toGuiElementType(int value)
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
                return boost::none;
        }
    }

    boost::optional<GuiElementType> extractGuiElementType(const TdfBlock& block, const std::string& key)
    {
        auto value = extractInt(block, key);
        if (!value)
        {
            return boost::none;
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

    boost::optional<GuiEntry> parseGuiEntry(const TdfBlock& e)
    {
        GuiEntry g;

        auto common = extractBlock(e, "COMMON");
        if (!common)
        {
            throw GuiParseException("Block is missing common section");
        }

        g.common.id = expectGuiElementType(*common, "id");
        g.common.assoc = expectInt(*common, "assoc");
        g.common.name = expectString(*common, "name");
        g.common.xpos = expectInt(*common, "xpos");
        g.common.ypos = expectInt(*common, "ypos");
        g.common.width = expectInt(*common, "width");
        g.common.height = expectInt(*common, "height");
        g.common.attribs = expectInt(*common, "attribs");
        g.common.colorf = expectInt(*common, "colorf");
        g.common.colorb = expectInt(*common, "colorb");
        g.common.textureNumber = expectInt(*common, "texturenumber");
        g.common.fontNumber = expectInt(*common, "fontnumber");
        g.common.active = expectBool(*common, "active");
        g.common.commonAttribs = expectInt(*common, "commonattribs");
        g.common.help = expectString(*common, "help");

        g.panel = extractString(e, "panel");
        g.crDefault = extractString(e, "crdefault");
        g.escdefault = extractString(e, "escdefault");
        g.defaultFocus = extractString(e, "defaultfocus");

        g.totalGadgets = extractInt(e, "totalgadgets");

        auto version = extractBlock(e, "VERSION");
        if (version)
        {
            auto major = expectInt(*version, "major");
            auto minor = expectInt(*version, "minor");
            auto revision = expectInt(*version, "revision");
            g.version = GuiVersion(major, minor, revision);
        }

        g.status = extractInt(e, "status");
        g.text = extractString(e, "text");
        g.quickKey = extractInt(e, "quickkey");
        g.grayedOut = extractBool(e, "grayedout");
        g.stages = extractInt(e, "stages");

        return g;
    }

    boost::optional<std::vector<GuiEntry>> parseGui(const TdfBlock& tdf)
    {
        std::vector<GuiEntry> entries;

        for (auto& entry : tdf)
        {
            auto block = boost::get<TdfBlock&>(*(entry.value));
            auto guiEntry = parseGuiEntry(block);
            if (!guiEntry)
            {
                return boost::none;
            }

            entries.push_back(*guiEntry);
        }

        return entries;
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
