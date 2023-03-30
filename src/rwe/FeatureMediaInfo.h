#pragma once

#include <memory>
#include <optional>
#include <rwe/render/SpriteSeries.h>
#include <string>
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

    struct FeatureMediaInfo
    {
        std::string world;
        std::string description;
        std::string category;

        FeatureRenderInfo renderInfo;

        std::string seqNameReclamate;

        std::string seqNameBurn;
        std::string seqNameBurnShad;

        std::string seqNameDie;
    };
}
