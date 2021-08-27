#pragma once

#include <optional>
#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    class GuiParseException : public std::runtime_error
    {
    public:
        explicit GuiParseException(const char* message);
        explicit GuiParseException(const std::string& message);
    };
    enum class GuiElementType
    {
        Panel = 0,
        Button = 1,
        ListBox = 2,
        TextBox = 3,
        ScrollBar = 4,
        Label = 5,
        Surface = 6,
        Font = 7,
        PictureBox = 12
    };

    struct GuiButtonAttrib
    {
        static constexpr int LabelDisplayLeft = 1;
        static constexpr int LabelDisplayCenter = 2;

        static constexpr int BehaviorRadio = 16;
        static constexpr int BehaviorBuildButton = 32;
        static constexpr int BehaviorToggle = 64;
        static constexpr int BehaviorCycle = 256;
    };

    const int GuiScrollHorizontalAttrib = 1;
    const int GuiScrollVerticalAttrib = 2;

    struct GuiVersion
    {
        int majorVersion;
        int minorVersion;
        int revision;

        GuiVersion() = default;
        GuiVersion(int major, int minor, int revision) : majorVersion(major), minorVersion(minor), revision(revision) {}

        bool operator==(const GuiVersion& rhs) const
        {
            return majorVersion == rhs.majorVersion
                && minorVersion == rhs.minorVersion
                && revision == rhs.revision;
        }

        bool operator!=(const GuiVersion& rhs) const
        {
            return !(rhs == *this);
        }
    };

    struct GuiCommon
    {
        GuiElementType id;

        /**
         * Indicates the "group" the element is associated with.
         * Items in the same group may pass messages to each other.
         * For example, a ScrollBar may indicate to a ListBox
         * in the same group that the user has scrolled up or down.
         */
        int assoc;

        std::string name;
        int xpos;
        int ypos;
        int width;
        int height;
        int attribs;
        int colorf;
        int colorb;
        int textureNumber;
        int fontNumber;
        bool active;
        int commonAttribs;
        std::optional<std::string> help;

        bool operator==(const GuiCommon& rhs) const
        {
            return id == rhs.id
                && assoc == rhs.assoc
                && name == rhs.name
                && xpos == rhs.xpos
                && ypos == rhs.ypos
                && width == rhs.width
                && height == rhs.height
                && attribs == rhs.attribs
                && colorf == rhs.colorf
                && colorb == rhs.colorb
                && textureNumber == rhs.textureNumber
                && fontNumber == rhs.fontNumber
                && active == rhs.active
                && commonAttribs == rhs.commonAttribs
                && help == rhs.help;
        }

        bool operator!=(const GuiCommon& rhs) const
        {
            return !(rhs == *this);
        }
    };

    struct GuiEntry
    {
        GuiCommon common;

        std::optional<std::string> panel;
        std::optional<std::string> crDefault;
        std::optional<std::string> escdefault;
        std::optional<std::string> defaultFocus;
        std::optional<int> totalGadgets;

        std::optional<GuiVersion> version;

        std::optional<int> status;
        std::optional<std::string> text;
        std::optional<int> quickKey;
        std::optional<bool> grayedOut;
        std::optional<int> stages;

        bool operator==(const GuiEntry& rhs) const;

        bool operator!=(const GuiEntry& rhs) const;
    };


    std::optional<std::vector<GuiEntry>> parseGui(const std::vector<TdfBlock>& tdf);
    std::optional<std::vector<GuiEntry>> parseGuiFromBytes(const std::vector<char>& bytes);
}
