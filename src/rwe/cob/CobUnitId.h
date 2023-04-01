#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct CobUnitIdTag;
    using CobUnitId = OpaqueId<unsigned int, CobUnitIdTag>;
}
