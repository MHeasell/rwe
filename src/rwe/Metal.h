#ifndef RWE_METAL_H
#define RWE_METAL_H

#include <rwe/OpaqueUnit.h>

namespace rwe
{
    struct MetalTag;
    using Metal = OpaqueUnit<float, MetalTag>;
}

#endif
