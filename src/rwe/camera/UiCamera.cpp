#include "UiCamera.h"

namespace rwe
{
    Matrix4f UiCamera::getViewMatrix() const
    {
        return Matrix4f::identity();
    }

    Matrix4f UiCamera::getInverseViewMatrix() const
    {
        return Matrix4f::identity();
    }

    Matrix4f UiCamera::getProjectionMatrix() const
    {
        return Matrix4f::orthographicProjection(0.0f, width, height, 0.0f, 100.0f, -100.0f);
    }

    Matrix4f UiCamera::getInverseProjectionMatrix() const
    {
        return Matrix4f::inverseOrthographicProjection(0.0f, width, height, 0.0f, 100.0f, -100.0f);
    }

    float UiCamera::getWidth()
    {
        return width;
    }

    float UiCamera::getHeight()
    {
        return height;
    }
}
