#ifndef RWE_CABINETCAMERA_H
#define RWE_CABINETCAMERA_H

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

        float width;
        float height;

        Vector3f position;

    public:
        CabinetCamera(float width, float height);

        Matrix4f getViewMatrix() const override;

        Matrix4f getInverseViewMatrix() const override;

        Matrix4f getProjectionMatrix() const override;

        Matrix4f getInverseProjectionMatrix() const override;

        float getWidth() const;

        float getHeight() const;

        Vector3f getPosition() const;

        const Vector3f& getRawPosition() const;

        void translate(const Vector3f& translation);

        void setPosition(const Vector3f& newPosition);
    };
}

#endif
