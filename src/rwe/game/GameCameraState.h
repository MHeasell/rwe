#pragma once

#include <rwe/math/Vector3f.h>
namespace rwe
{
    struct GameCameraState
    {
        float zoom{1.0f};
        Vector3f position{0.0f, 0.0f, 0.0f};

        Vector3f getRoundedPosition() const;

        float scaleDimension(float dimension) const;
    };
}
