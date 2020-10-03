#pragma once
#include <rwe/tdf/TdfBlock.h>
#include <rwe/fbi/UnitFbi.h>

namespace rwe
{
    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf);

    UnitFbi parseUnitFbi(const TdfBlock& tdf);
}
