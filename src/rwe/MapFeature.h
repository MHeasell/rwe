#pragma once

#include <memory>
#include <optional>
#include <rwe/SpriteSeries.h>
#include <rwe/math/Vector3f.h>

namespace rwe
{
    struct MapFeature
    {
        std::shared_ptr<SpriteSeries> animation;
        bool transparentAnimation;
        std::optional<std::shared_ptr<SpriteSeries>> shadowAnimation;
        bool transparentShadow;
        Vector3f position;
        int footprintX;
        int footprintZ;
        float height;
        bool isBlocking;
        bool isIndestructible;
        unsigned int metal;

        bool isStanding() const;
    };
}
