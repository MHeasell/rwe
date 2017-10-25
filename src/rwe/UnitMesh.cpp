#include "UnitMesh.h"

namespace rwe
{

    void UnitMesh::render(
        GraphicsContext& context,
        ShaderProgramIdentifier textureShader,
        ShaderProgramIdentifier colorShader,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix) const
    {
        auto matrix = modelMatrix * Matrix4f::translation(origin);

        if (visible)
        {
            context.drawShaderMesh(*mesh, textureShader, colorShader, matrix, viewMatrix, projectionMatrix);
        }

        for (const auto& c : children)
        {
            c.render(context, textureShader, colorShader, matrix, viewMatrix, projectionMatrix);
        }
    }

    boost::optional<UnitMesh&> UnitMesh::find(const std::string& pieceName)
    {
        if (pieceName == name)
        {
            return *this;
        }

        for (auto& c : children)
        {
            auto piece = c.find(pieceName);
            if (piece)
            {
                return piece;
            }
        }

        return boost::none;
    }
}
