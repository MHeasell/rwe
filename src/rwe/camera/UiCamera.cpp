#include "UiCamera.h"

namespace rwe
{
    Matrix4f UiCamera::getViewMatrix() const
    {
        return Matrix4f::identity();
    }

    Matrix4f UiCamera::getProjectionMatrix() const
    {
        return Matrix4f::orthographicProjection(0.0f, width, height, 0.0f, 100.0f, -100.0f);
    }
}
