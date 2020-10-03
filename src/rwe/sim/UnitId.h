#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct UnitIdTag;
    using UnitId = OpaqueId<unsigned int, UnitIdTag>;
}
