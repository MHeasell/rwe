#include "Unit.h"

namespace rwe
{
    Unit::Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment) : mesh(mesh), cobEnvironment(std::move(cobEnvironment))
    {
    }

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
