#ifndef RWE_GUI_H
#define RWE_GUI_H

#include <string>

namespace rwe
{
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

    struct GuiEntry
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
        std::string help;
    };
}

#endif
