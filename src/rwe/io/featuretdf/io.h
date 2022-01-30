#pragma once

#include "FeatureTdf.h"
#include <rwe/io/tdf/TdfBlock.h>

namespace rwe
{
    FeatureTdf parseFeatureDefinition(const TdfBlock& tdf);
}
