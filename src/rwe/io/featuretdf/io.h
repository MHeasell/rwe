#pragma once

#include <rwe/io/featuretdf/FeatureTdf.h>
#include <rwe/io/tdf/TdfBlock.h>

namespace rwe
{
    FeatureTdf parseFeatureTdf(const TdfBlock& tdf);
}
