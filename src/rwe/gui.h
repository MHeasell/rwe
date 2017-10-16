#ifndef RWE_GUI_H
#define RWE_GUI_H

#include <boost/optional.hpp>
#include <rwe/tdf/TdfBlock.h>
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
        boost::optional<std::string> help;

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

        boost::optional<std::string> panel;
        boost::optional<std::string> crDefault;
        boost::optional<std::string> escdefault;
        boost::optional<std::string> defaultFocus;
        boost::optional<int> totalGadgets;

        boost::optional<GuiVersion> version;

        boost::optional<int> status;
        boost::optional<std::string> text;
        boost::optional<int> quickKey;
        boost::optional<bool> grayedOut;
        boost::optional<int> stages;

        bool operator==(const GuiEntry& rhs) const;

        bool operator!=(const GuiEntry& rhs) const;
    };


    boost::optional<std::vector<GuiEntry>> parseGui(const TdfBlock& tdf);
}

#endif
