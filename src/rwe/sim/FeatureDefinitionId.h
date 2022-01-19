#pragma once

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct FeatureDefinitionIdTag;
    using FeatureDefinitionId = OpaqueId<unsigned int, FeatureDefinitionIdTag>;
}
