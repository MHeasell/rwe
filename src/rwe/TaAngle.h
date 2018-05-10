#ifndef RWE_TAANGLE_H
#define RWE_TAANGLE_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct TaAngleTag;
    using TaAngle = OpaqueId<uint16_t, TaAngleTag>;
}

#endif
