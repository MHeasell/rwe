#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct MovementClassIdTag;
    using MovementClassId = OpaqueId<int, MovementClassIdTag>;
}
