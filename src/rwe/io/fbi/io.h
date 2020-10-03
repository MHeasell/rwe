#pragma once
#include <rwe/io/fbi/UnitFbi.h>
#include <rwe/io/tdf/TdfBlock.h>

namespace rwe
{
    UnitFbi parseUnitInfoBlock(const TdfBlock& tdf);

    UnitFbi parseUnitFbi(const TdfBlock& tdf);
}
