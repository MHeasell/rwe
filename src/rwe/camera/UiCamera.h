#ifndef RWE_UICAMERA_H
#define RWE_UICAMERA_H

#include <rwe/camera/AbstractCamera.h>

namespace rwe
{
    class UiCamera : public AbstractCamera
    {
    private:
        float width;
        float height;
        Matrix4f projectionMatrix;
        Matrix4f inverseProjectionMatrix;

    public:
        UiCamera(float width, float height);

        float getWidth();

        float getHeight();

        const Matrix4f& getViewProjectionMatrix() const override;

        const Matrix4f& getInverseViewProjectionMatrix() const override;
    };
}

#endif
