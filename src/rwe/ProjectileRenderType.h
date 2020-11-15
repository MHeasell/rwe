#pragma once

#include <memory>
#include <rwe/math/Vector3f.h>
#include <rwe/render/SpriteSeries.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/UnitMesh.h>
#include <variant>

namespace rwe
{
    struct ProjectileRenderTypeLaser
    {
        Vector3f color;
        Vector3f color2;

        /** Duration in ticks */
        SimScalar duration;
    };

    struct ProjectileRenderTypeModel
    {
        enum class RotationMode
        {
            None,
            HalfZ,
            QuarterY,
        };
        std::string objectName;
        std::shared_ptr<UnitMesh> mesh;
        RotationMode rotationMode;
    };

    struct ProjectileRenderTypeMindgun
    {
    };

    struct ProjectileRenderTypeSprite
    {
        std::string gaf;
        std::string anim;
    };

    struct ProjectileRenderTypeFlamethrower
    {
    };

    struct ProjectileRenderTypeLightning
    {
        Vector3f color;

        /** Duration in ticks */
        SimScalar duration;
    };

    using ProjectileRenderType = std::variant<
        ProjectileRenderTypeLaser,
        ProjectileRenderTypeModel,
        ProjectileRenderTypeMindgun,
        ProjectileRenderTypeSprite,
        ProjectileRenderTypeFlamethrower,
        ProjectileRenderTypeLightning>;
}
