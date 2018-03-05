#ifndef RWE_MOVEMENTCLASSID_H
#define RWE_MOVEMENTCLASSID_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct MovementClassIdTag;
    using MovementClassId = OpaqueId<unsigned int, MovementClassIdTag>;
}

#endif
