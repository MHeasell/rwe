#include "FeatureDefinition.h"

namespace rwe
{
    static const SimScalar StandingFeatureMinHeight = 10_ss;

    bool FeatureDefinition::isStanding() const
    {
        return height >= StandingFeatureMinHeight;
    }
}
