#include "AbstractCamera.h"

namespace rwe
{
    Matrix4f AbstractCamera::getViewProjectionMatrix() const
    {
        return getProjectionMatrix() * getViewMatrix();
    }
}
