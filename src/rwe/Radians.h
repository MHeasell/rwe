#ifndef RWE_RADIANS_H
#define RWE_RADIANS_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct RadiansTag;
    using Radians = OpaqueId<float, RadiansTag>;
}

#endif
