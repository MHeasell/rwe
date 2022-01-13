#pragma once

#include <rwe/sim/SimVector.h>
#include <string>

namespace rwe
{
    struct MapFeature
    {
        std::string featureName;
        SimVector position;
    };
}
