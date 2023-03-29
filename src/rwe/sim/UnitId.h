#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct UnitIdTag;
    using UnitId = OpaqueId<unsigned int, UnitIdTag>;
}
