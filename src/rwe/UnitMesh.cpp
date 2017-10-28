#include "UnitMesh.h"
#include "util.h"

namespace rwe
{
    void applyMoveOperation(boost::optional<UnitMesh::MoveOperation>& op, float& currentPos, float dt)
    {
        if (op)
        {
            float remaining = op->targetPosition - currentPos;
            float frameSpeed = op->speed * dt;
            if (std::abs(remaining) <= frameSpeed)
            {
                currentPos = op->targetPosition;
                op = boost::none;
            }
            else
            {
                currentPos += frameSpeed * (remaining > 0.0f ? 1.0f : -1.0f);
            }
        }
    }

    void UnitMesh::render(
        GraphicsContext& context,
        ShaderProgramIdentifier textureShader,
        ShaderProgramIdentifier colorShader,
        const Matrix4f& modelMatrix,
        const Matrix4f& viewMatrix,
        const Matrix4f& projectionMatrix) const
    {
        auto matrix = Matrix4f::translation(origin + offset) * modelMatrix;

        if (visible)
        {
            context.drawShaderMesh(*mesh, textureShader, colorShader, matrix, viewMatrix, projectionMatrix);
        }

        for (const auto& c : children)
        {
            c.render(context, textureShader, colorShader, matrix, viewMatrix, projectionMatrix);
        }
    }

    boost::optional<const UnitMesh&> UnitMesh::find(const std::string& pieceName) const
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

    boost::optional<UnitMesh&> UnitMesh::find(const std::string& pieceName)
    {
        auto value = static_cast<const UnitMesh&>(*this).find(pieceName);
        if (!value)
        {
            return boost::none;
        }

        return const_cast<UnitMesh&>(*value);
    }

    void UnitMesh::update(float dt)
    {
        applyMoveOperation(xMoveOperation, offset.x, dt);
        applyMoveOperation(yMoveOperation, offset.y, dt);
        applyMoveOperation(zMoveOperation, offset.z, dt);
    }

    UnitMesh::MoveOperation::MoveOperation(float targetPosition, float speed)
        : targetPosition(targetPosition), speed(speed)
    {
    }
}
