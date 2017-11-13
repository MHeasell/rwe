#ifndef RWE_UNITID_H
#define RWE_UNITID_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct UnitIdTag;
    using UnitId = OpaqueId<unsigned int, UnitIdTag>;
}

#endif
