#pragma once

#include <rwe/grid/DiscreteRect.h>
#include <rwe/io/tdf/TdfBlock.h>
#include <string>

namespace rwe
{
    struct SideDataRect
    {
        int x1;
        int y1;
        int x2;
        int y2;

        SideDataRect() = default;
        SideDataRect(int x1, int y1, int x2, int y2);

        bool operator==(const SideDataRect& rhs) const;

        bool operator!=(const SideDataRect& rhs) const;

        DiscreteRect toDiscreteRect() const;
    };

    struct SideData
    {
        std::string name;
        std::string namePrefix;
        std::string commander;
        std::string intGaf;

        std::string font;
        std::string fontGui;
        int energyColor;
        int metalColor;

        SideDataRect logo;
        SideDataRect energyBar;
        SideDataRect energyNum;
        SideDataRect energyMax;
        SideDataRect energy0;
        SideDataRect metalBar;
        SideDataRect metalNum;
        SideDataRect metalMax;
        SideDataRect metal0;
        SideDataRect totalUnits;
        SideDataRect totalTime;
        SideDataRect energyProduced;
        SideDataRect energyConsumed;
        SideDataRect metalProduced;
        SideDataRect metalConsumed;

        // all of these region describe areas on the screen footer

        SideDataRect logo2;
        SideDataRect unitName;
        SideDataRect damageBar;
        SideDataRect unitMetalMake;
        SideDataRect unitMetalUse;
        SideDataRect unitEnergyMake;
        SideDataRect unitEnergyUse;
        SideDataRect missionText;
        SideDataRect unitName2;
        SideDataRect damageBar2;
        SideDataRect _name;
        SideDataRect description;
        SideDataRect reload1;
        SideDataRect reload2;
        SideDataRect reload3;
    };

    SideData parseSideData(const TdfBlock& tdf);

    std::vector<SideData> parseSidesFromSideData(const TdfBlock& tdf);
}
