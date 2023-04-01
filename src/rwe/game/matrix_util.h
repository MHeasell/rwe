#pragma once

#include <rwe/math/Matrix4f.h>
#include <rwe/sim/SimScalar.h>

namespace rwe
{
    Matrix4f toFloatMatrix(Matrix4x<SimScalar> m);

}
