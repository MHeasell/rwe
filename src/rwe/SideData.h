#ifndef RWE_SIDEDATA_H
#define RWE_SIDEDATA_H

#include <string>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    struct SideDataRect
    {
        unsigned int x1;
        unsigned int y1;
        unsigned int x2;
        unsigned int y2;

        SideDataRect() = default;
        SideDataRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

        bool operator==(const SideDataRect& rhs) const;

        bool operator!=(const SideDataRect& rhs) const;
    };

    struct SideData
    {
        std::string name;
        std::string namePrefix;
        std::string commander;
        std::string intGaf;

        std::string font;
        std::string fontGui;
        unsigned int energyColor;
        unsigned int metalColor;

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

#endif
