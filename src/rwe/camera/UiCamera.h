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

    public:
        UiCamera(float width, float height) : width(width), height(height) {}

        Matrix4f getViewMatrix() const override;

        Matrix4f getProjectionMatrix() const override;

        float getWidth();

        float getHeight();
    };
}

#endif
