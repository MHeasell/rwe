#pragma once

#include "FeatureDefinition.h"
#include <rwe/io/tdf/TdfBlock.h>

namespace rwe
{
    FeatureDefinition parseFeatureDefinition(const TdfBlock& tdf);
}
