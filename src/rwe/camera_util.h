#pragma once

#include <rwe/geometry/Ray3f.h>
#include <rwe/math/Matrix4f.h>
#include <rwe/math/Vector2f.h>

namespace rwe
{
    Ray3f screenToWorldRayUtil(const Matrix4f& inverseViewProjectionMatrix, const Vector2f& point);
}
