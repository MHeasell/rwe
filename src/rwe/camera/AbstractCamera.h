#ifndef RWE_ABSTRACTCAMERA_H
#define RWE_ABSTRACTCAMERA_H

#include <rwe/math/Matrix4f.h>

namespace rwe
{
    class AbstractCamera
    {
    public:
        virtual ~AbstractCamera() = default;

        AbstractCamera() = default;

        AbstractCamera(const AbstractCamera& c) = default;
        AbstractCamera& operator=(const AbstractCamera& c) = default;
        AbstractCamera(AbstractCamera&& c) = default;
        AbstractCamera& operator=(AbstractCamera&& c) = default;

        virtual Matrix4f getViewMatrix() const = 0;

        virtual Matrix4f getProjectionMatrix() const = 0;

        Matrix4f getViewProjectionMatrix() const;
    };
}

#endif
