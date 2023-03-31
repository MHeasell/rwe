#include "SideData.h"

namespace rwe
{
    SideDataRect::SideDataRect(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2)
        : x1(x1), y1(y1), x2(x2), y2(y2)
    {
    }

    bool SideDataRect::operator==(const SideDataRect& rhs) const
    {
        return x1 == rhs.x1 && y1 == rhs.y1 && x2 == rhs.x2 && y2 == rhs.y2;
    }

    bool SideDataRect::operator!=(const SideDataRect& rhs) const
    {
        return !(rhs == *this);
    }

    DiscreteRect SideDataRect::toDiscreteRect() const
    {
        unsigned int minX;
        unsigned int maxX;
        if (x1 < x2)
        {
            minX = x1;
            maxX = x2;
        }
        else
        {
            minX = x2;
            maxX = x1;
        }

        unsigned int minY;
        unsigned int maxY;
        if (y1 < y2)
        {
            minY = y1;
            maxY = y2;
        }
        else
        {
            minY = y2;
            maxY = y1;
        }

        return DiscreteRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
    }

    SideDataRect parseSideDataRect(const TdfBlock& t)
    {
        SideDataRect s;
        s.x1 = t.expectUint("x1");
        s.y1 = t.expectUint("y1");
        s.x2 = t.expectUint("x2");
        s.y2 = t.expectUint("y2");
        return s;
    }

    SideDataRect expectSideDataRect(const TdfBlock& t, const std::string& name)
    {
        auto b = t.findBlock(name);
        if (!b)
        {
            throw TdfValueException("Failed to find block with name: " + name);
        }

        return parseSideDataRect(*b);
    }

    SideData parseSideData(const TdfBlock& t)
    {
        SideData s;
        s.name = t.expectString("name");
        s.namePrefix = t.expectString("nameprefix");
        s.commander = t.expectString("commander");
        s.intGaf = t.expectString("intgaf");

        s.font = t.expectString("font");
        s.fontGui = t.expectString("fontgui");
        s.energyColor = t.expectUint("energycolor");
        s.metalColor = t.expectUint("metalcolor");

        s.logo = expectSideDataRect(t, "LOGO");
        s.energyBar = expectSideDataRect(t, "ENERGYBAR");
        s.energyNum = expectSideDataRect(t, "ENERGYNUM");
        s.energyMax = expectSideDataRect(t, "ENERGYMAX");
        s.energy0 = expectSideDataRect(t, "ENERGY0");
        s.metalBar = expectSideDataRect(t, "METALBAR");
        s.metalNum = expectSideDataRect(t, "METALNUM");
        s.metalMax = expectSideDataRect(t, "METALMAX");
        s.metal0 = expectSideDataRect(t, "METAL0");
        s.totalUnits = expectSideDataRect(t, "TOTALUNITS");
        s.totalTime = expectSideDataRect(t, "TOTALTIME");
        s.energyProduced = expectSideDataRect(t, "ENERGYPRODUCED");
        s.energyConsumed = expectSideDataRect(t, "ENERGYCONSUMED");
        s.metalProduced = expectSideDataRect(t, "METALPRODUCED");
        s.metalConsumed = expectSideDataRect(t, "METALCONSUMED");

        s.logo2 = expectSideDataRect(t, "LOGO2");
        s.unitName = expectSideDataRect(t, "UNITNAME");
        s.damageBar = expectSideDataRect(t, "DAMAGEBAR");
        s.unitMetalMake = expectSideDataRect(t, "UNITMETALMAKE");
        s.unitMetalUse = expectSideDataRect(t, "UNITMETALUSE");
        s.unitEnergyMake = expectSideDataRect(t, "UNITENERGYMAKE");
        s.unitEnergyUse = expectSideDataRect(t, "UNITENERGYUSE");
        s.missionText = expectSideDataRect(t, "MISSIONTEXT");
        s.unitName2 = expectSideDataRect(t, "UNITNAME2");
        s.damageBar2 = expectSideDataRect(t, "DAMAGEBAR2");
        s._name = expectSideDataRect(t, "NAME");
        s.description = expectSideDataRect(t, "DESCRIPTION");
        s.reload1 = expectSideDataRect(t, "RELOAD1");
        s.reload2 = expectSideDataRect(t, "RELOAD2");
        s.reload3 = expectSideDataRect(t, "RELOAD3");

        return s;
    }

    std::vector<SideData> parseSidesFromSideData(const TdfBlock& tdf)
    {
        std::vector<SideData> list;

        unsigned int i = 0;
        for (;;)
        {
            auto block = tdf.findBlock("SIDE" + std::to_string(i));
            if (!block)
            {
                return list;
            }

            list.push_back(parseSideData(*block));

            ++i;
        }
    }
}
