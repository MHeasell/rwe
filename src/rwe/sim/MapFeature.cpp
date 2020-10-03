#include "MapFeature.h"

namespace rwe
{
    static const SimScalar StandingFeatureMinHeight = 10_ss;

    bool MapFeature::isStanding() const
    {
        return height >= StandingFeatureMinHeight;
    }
}
