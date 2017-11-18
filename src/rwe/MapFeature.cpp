#include "MapFeature.h"

namespace rwe
{
    static const float StandingFeatureMinHeight = 5.0f;

    bool MapFeature::isBlocking() const
    {
        return height >= StandingFeatureMinHeight;
    }
}
