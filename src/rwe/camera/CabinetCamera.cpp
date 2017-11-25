#include "CabinetCamera.h"
#include <cmath>

namespace rwe
{
    const Vector3f CabinetCamera::_forward(0.0f, 1.0f, 0.0f);
    const Vector3f CabinetCamera::_up(0.0f, 0.0f, -1.0f);
    const Vector3f CabinetCamera::_side(1.0f, 0.0f, 0.0f);

    CabinetCamera::CabinetCamera(float width, float height) : width(width), height(height), position(0.0f, 0.0f, 0.0f)
    {
    }

    Matrix4f CabinetCamera::getViewMatrix() const
    {
        auto translation = Matrix4f::translation(-getPosition());
        auto rotation = Matrix4f::rotationToAxes(_side, _up, _forward);
        return rotation * translation;
    }

    Matrix4f CabinetCamera::getInverseViewMatrix() const
    {
        auto translation = Matrix4f::translation(getPosition());
        auto rotation = Matrix4f::rotationToAxes(_side, _up, _forward).transposed();
        return translation * rotation;
    }

    Matrix4f CabinetCamera::getProjectionMatrix() const
    {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;

        auto cabinet = Matrix4f::cabinetProjection(0.0f, 0.5f);

        auto ortho = Matrix4f::orthographicProjection(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            -1000.0f,
            1000.0f);

        return ortho * cabinet;
    }

    Matrix4f CabinetCamera::getInverseProjectionMatrix() const
    {
        float halfWidth = width / 2.0f;
        float halfHeight = height / 2.0f;

        auto inverseCabinet = Matrix4f::cabinetProjection(0.0f, -0.5f);

        auto inverseOrtho = Matrix4f::inverseOrthographicProjection(
            -halfWidth,
            halfWidth,
            -halfHeight,
            halfHeight,
            -1000.0f,
            1000.0f);

        return inverseCabinet * inverseOrtho;
    }

    float CabinetCamera::getWidth() const
    {
        return width;
    }

    float CabinetCamera::getHeight() const
    {
        return height;
    }

    Vector3f CabinetCamera::getPosition() const
    {
        return Vector3f(
            std::round(position.x),
            std::round(position.y),
            std::round(position.z));
    }

    const Vector3f& CabinetCamera::getRawPosition() const
    {
        return position;
    }

    void CabinetCamera::translate(const Vector3f& translation)
    {
        position += translation;
    }

    void CabinetCamera::setPosition(const Vector3f& newPosition)
    {
        position = newPosition;
    }
}
