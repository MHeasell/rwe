#pragma once

#include <memory>
#include <optional>
#include <rwe/render/SpriteSeries.h>
#include <rwe/sim/SimVector.h>
#include <variant>

namespace rwe
{
    struct FeatureSpriteInfo
    {
        std::shared_ptr<SpriteSeries> animation;
        bool transparentAnimation;
        std::optional<std::shared_ptr<SpriteSeries>> shadowAnimation;
        bool transparentShadow;
    };

    struct FeatureObjectInfo
    {
        std::string objectName;
    };

    using FeatureRenderInfo = std::variant<FeatureSpriteInfo, FeatureObjectInfo>;

    struct MapFeature
    {
        FeatureRenderInfo renderInfo;
        SimVector position;
        int footprintX;
        int footprintZ;
        SimScalar height;
        bool isBlocking;
        bool isIndestructible;
        int metal;

        bool isStanding() const;
    };
}
