#pragma once

#include <memory>
#include <rwe/ShaderMesh.h>
#include <rwe/SimScalar.h>
#include <rwe/UnitMesh.h>
#include <rwe/math/Vector3f.h>
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
        std::shared_ptr<UnitMesh> mesh;
        RotationMode rotationMode;
    };

    struct ProjectileRenderTypeMindgun
    {
    };

    struct ProjectileRenderTypeSprite
    {
        enum class Sprite
        {
            CannonShell,
            PlasmaSm,
            PlasmaMd,
            UltraShell,
            Nothing
        };

        Sprite sprite;
    };

    struct ProjectileRenderTypeFlamethrower
    {
    };

    struct ProjectileRenderTypeLightning
    {
        Vector3f color;
        Vector3f color2;

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
