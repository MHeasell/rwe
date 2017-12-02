#include "UiCamera.h"

namespace rwe
{
    UiCamera::UiCamera(float width, float height)
        : width(width),
          height(height),
          projectionMatrix(Matrix4f::orthographicProjection(0.0f, width, height, 0.0f, 100.0f, -100.0f)),
          inverseProjectionMatrix(Matrix4f::inverseOrthographicProjection(0.0f, width, height, 0.0f, 100.0f, -100.0f))
    {
    }

    float UiCamera::getWidth()
    {
        return width;
    }

    float UiCamera::getHeight()
    {
        return height;
    }

    const Matrix4f& UiCamera::getViewProjectionMatrix() const
    {
        return projectionMatrix;
    }

    const Matrix4f& UiCamera::getInverseViewProjectionMatrix() const
    {
        return inverseProjectionMatrix;
    }
}
