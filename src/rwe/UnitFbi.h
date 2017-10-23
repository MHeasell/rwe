#ifndef RWE_UNITFBI_H
#define RWE_UNITFBI_H

#include <string>
#include <rwe/tdf/TdfBlock.h>

namespace rwe
{
    struct UnitFbi
    {
        std::string unitName;
        std::string objectName;
    };

    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf);

    UnitFbi parseUnitFbi(const TdfBlock& tdf);
}

#endif
