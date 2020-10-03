#pragma once

#include <memory>
#include <optional>
#include <rwe/SpriteSeries.h>
#include <rwe/sim/SimVector.h>

namespace rwe
{
    struct MapFeature
    {
        std::shared_ptr<SpriteSeries> animation;
        bool transparentAnimation;
        std::optional<std::shared_ptr<SpriteSeries>> shadowAnimation;
        bool transparentShadow;
        SimVector position;
        int footprintX;
        int footprintZ;
        SimScalar height;
        bool isBlocking;
        bool isIndestructible;
        unsigned int metal;

        bool isStanding() const;
    };
}
