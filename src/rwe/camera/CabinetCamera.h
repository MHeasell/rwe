#pragma once

#include <rwe/camera/AbstractCamera.h>

namespace rwe
{
    class CabinetCamera : public AbstractCamera
    {
    private:
        static const Vector3f _direction;
        static const Vector3f _side;
        static const Vector3f _up;
        static const Vector3f _forward;

        static Matrix4f computeViewProjection(const Vector3f& position, float width, float height);
        static Matrix4f computeProjection(float width, float height);
        static Matrix4f computeView(const Vector3f& position);
        static Matrix4f computeInverseViewProjection(const Vector3f& position, float width, float height);
        static Matrix4f computeInverseProjection(float width, float height);
        static Matrix4f computeInverseView(const Vector3f& position);

        float width;
        float height;

        Vector3f position;

        Matrix4f viewProjection;
        Matrix4f inverseViewProjection;

    public:
        CabinetCamera(float width, float height);

        float getWidth() const;

        float getHeight() const;

        Vector3f getPosition() const;

        const Vector3f& getRawPosition() const;

        void translate(const Vector3f& translation);

        void setPosition(const Vector3f& newPosition);

        void setPositionXZ(const Vector2f& newPosition);

        const Matrix4f& getViewProjectionMatrix() const override;

        const Matrix4f& getInverseViewProjectionMatrix() const override;

    private:
        void updateCachedMatrices();
    };
}
