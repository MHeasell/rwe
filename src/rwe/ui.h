#ifndef RWE_UI_H
#define RWE_UI_H

#include <rwe/ui/UiPanel.h>
#include <rwe/gui.h>

namespace rwe
{
    UiPanel panelFromGuiFile(const std::vector<GuiEntry>& entries)
    {
        // first entry sets up the panel
        assert(entries.size() > 0);
        auto panelEntry = entries[0];

//        UiPanel panel(
//                panelEntry.common.xpos,
//                panelEntry.common.ypos,
//                panelEntry.common.width,
//                panelEntry.common.height,
//                entries.
//        )
    }
}

#endif
