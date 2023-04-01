#include "matrix_util.h"

namespace rwe
{
    Matrix4f toFloatMatrix(Matrix4x<SimScalar> m)
    {
        Matrix4f n;

        for (int i = 0; i < 16; ++i)
        {
            n.data[i] = simScalarToFloat(m.data[i]);
        }

        return n;
    }
}
