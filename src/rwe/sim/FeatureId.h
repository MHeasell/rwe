#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct FeatureIdTag;
    using FeatureId = OpaqueId<int, FeatureIdTag>;
}
