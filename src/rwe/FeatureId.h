#ifndef RWE_FEATUREID_H
#define RWE_FEATUREID_H

#include <rwe/OpaqueId.h>

namespace rwe
{
    struct FeatureIdTag;
    using FeatureId = OpaqueId<unsigned int, FeatureIdTag>;
}

#endif
