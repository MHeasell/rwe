#include "camera_util.h"
#include <rwe/math/Vector3f.h>

namespace rwe
{
    Ray3f screenToWorldRayUtil(const Matrix4f& inverseViewProjectionMatrix, const Vector2f& point)
    {
        auto startPoint = inverseViewProjectionMatrix * Vector3f(point.x, point.y, -1.0f);
        auto endPoint = inverseViewProjectionMatrix * Vector3f(point.x, point.y, 1.0f);
        auto direction = endPoint - startPoint;
        return Ray3f(startPoint, direction);
    }
}
