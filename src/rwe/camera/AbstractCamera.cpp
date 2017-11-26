#include "AbstractCamera.h"
#include <rwe/geometry/Ray3f.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    void AbstractCamera::getFrustum(std::vector<Vector3f>& list) const
    {
        // transform from clip space back to world space
        const auto& transform = getInverseViewProjectionMatrix();

        // near
        list.push_back(transform * Vector3f(-1.0f, 1.0f, -1.0f));  // top-left
        list.push_back(transform * Vector3f(1.0f, 1.0f, -1.0f));   // top-right
        list.push_back(transform * Vector3f(-1.0f, -1.0f, -1.0f)); // bottom-left
        list.push_back(transform * Vector3f(1.0f, -1.0f, -1.0f));  // bottom-right

        // far
        list.push_back(transform * Vector3f(-1.0f, 1.0f, 1.0f));  // top-left
        list.push_back(transform * Vector3f(1.0f, 1.0f, 1.0f));   // top-right
        list.push_back(transform * Vector3f(-1.0f, -1.0f, 1.0f)); // bottom-left
        list.push_back(transform * Vector3f(1.0f, -1.0f, 1.0f));  // bottom-right
    }

    Ray3f AbstractCamera::screenToWorldRay(const Vector2f& point) const
    {
        const auto& transform = getInverseViewProjectionMatrix();
        auto startPoint = transform * Vector3f(point.x, point.y, -1.0f);
        auto endPoint = transform * Vector3f(point.x, point.y, 1.0f);
        auto direction = endPoint - startPoint;
        return Ray3f(startPoint, direction);
    }
}
