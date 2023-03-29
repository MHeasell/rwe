#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct FeatureIdTag;
    using FeatureId = OpaqueId<unsigned int, FeatureIdTag>;
}
