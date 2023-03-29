#pragma once

#include <rwe/util/OpaqueId.h>

namespace rwe
{
    struct FeatureDefinitionIdTag;
    using FeatureDefinitionId = OpaqueId<unsigned int, FeatureDefinitionIdTag>;
}
