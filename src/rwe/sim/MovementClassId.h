#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct MovementClassIdTag;
    using MovementClassId = OpaqueId<unsigned int, MovementClassIdTag>;
}
