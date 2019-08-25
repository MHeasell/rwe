#pragma once

#include <rwe/SimScalar.h>
#include <rwe/math/Matrix4f.h>

namespace rwe
{
    Matrix4f toFloatMatrix(Matrix4x<SimScalar> m);

}
