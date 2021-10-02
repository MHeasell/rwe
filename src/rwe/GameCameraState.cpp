#include "GameCameraState.h"

namespace rwe
{
    Vector3f GameCameraState::getRoundedPosition() const
    {
        return Vector3f(
            std::round(position.x),
            std::round(position.y),
            std::round(position.z));
    }

    float GameCameraState::scaleDimension(float dimension) const
    {
        return dimension / zoom;
    }
}
