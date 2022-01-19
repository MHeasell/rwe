#pragma once

#include <rwe/sim/FeatureDefinitionId.h>
#include <rwe/sim/SimVector.h>
#include <string>

namespace rwe
{
    struct MapFeature
    {
        FeatureDefinitionId featureName;
        SimVector position;
    };
}
