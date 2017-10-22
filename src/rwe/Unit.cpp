#include "Unit.h"

namespace rwe
{
    void Unit::render(
        GraphicsContext& context,
        ShaderProgramIdentifier textureShader,
        ShaderProgramIdentifier colorShader,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix) const
    {
        auto matrix = Matrix4f::translation(position);
        mesh.render(context, textureShader, colorShader, matrix, viewMatrix, projectionMatrix);
    }
}
