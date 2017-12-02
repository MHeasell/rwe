#ifndef RWE_ABSTRACTCAMERA_H
#define RWE_ABSTRACTCAMERA_H

#include <rwe/geometry/Ray3f.h>
#include <rwe/math/Matrix4f.h>
#include <rwe/math/Vector2f.h>
#include <rwe/math/Vector3f.h>

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

        virtual const Matrix4f& getViewProjectionMatrix() const = 0;

        virtual const Matrix4f& getInverseViewProjectionMatrix() const = 0;

        /**
         * Returns the 8 points defining the frustum volume.
         */
        void getFrustum(std::vector<Vector3f>& list) const;

        /**
         * Transforms a point on the screen (in normalized device coordinates)
         * to a ray in world space shooting into the world from that point.
         */
        Ray3f screenToWorldRay(const Vector2f& point) const;
    };
}

#endif
