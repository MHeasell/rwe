#include "MapFeature.h"

namespace rwe
{
    static const float StandingFeatureMinHeight = 10.0f;

    bool MapFeature::isStanding() const
    {
        return height >= StandingFeatureMinHeight;
    }
}
