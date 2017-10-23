#include "UnitFbi.h"

namespace rwe
{
    UnitFbi parseUnitFbi(const TdfBlock& tdf)
    {
        auto infoBlock = tdf.findBlock("UNITINFO");
        if (!infoBlock)
        {
            throw std::runtime_error("FBI missing UNITINFO block");
        }

        return parseUnitInfoBlock(*infoBlock);
    }

    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf)
    {
        UnitFbi u;

        u.unitName = tdf.expectString("UnitName");
        u.objectName = tdf.expectString("Objectname");

        return u;
    }
}
