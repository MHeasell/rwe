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

    void Unit::moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::MoveOperation op(targetPosition, speed);

        switch (axis)
        {
            case Axis::X:
                piece->xMoveOperation = op;
                break;
            case Axis::Y:
                piece->yMoveOperation = op;
                break;
            case Axis::Z:
                piece->zMoveOperation = op;
                break;
        }
    }

    void Unit::moveObjectNow(const std::string& pieceName, Axis axis, float targetPosition)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->offset.x = targetPosition;
                piece->xMoveOperation = boost::none;
                break;
            case Axis::Y:
                piece->offset.y = targetPosition;
                piece->yMoveOperation = boost::none;
                break;
            case Axis::Z:
                piece->offset.z = targetPosition;
                piece->zMoveOperation = boost::none;
                break;
        }
    }

    void Unit::turnObject(const std::string& pieceName, Axis axis, float targetAngle, float speed)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        UnitMesh::TurnOperation op(toRadians(targetAngle), toRadians(speed));

        switch (axis)
        {
            case Axis::X:
                piece->xTurnOperation = op;
                break;
            case Axis::Y:
                piece->yTurnOperation = op;
                break;
            case Axis::Z:
                piece->zTurnOperation = op;
                break;
        }
    }

    void Unit::turnObjectNow(const std::string& pieceName, Axis axis, float targetAngle)
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name: " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                piece->rotation.x = toRadians(targetAngle);
                piece->xTurnOperation = boost::none;
                break;
            case Axis::Y:
                piece->rotation.y = toRadians(targetAngle);
                piece->yTurnOperation = boost::none;
                break;
            case Axis::Z:
                piece->rotation.z = toRadians(targetAngle);
                piece->zTurnOperation = boost::none;
                break;
        }
    }

    bool Unit::isMoveInProgress(const std::string& pieceName, Axis axis) const
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                return !!(piece->xMoveOperation);
            case Axis::Y:
                return !!(piece->yMoveOperation);
            case Axis::Z:
                return !!(piece->zMoveOperation);
        }

        throw std::logic_error("Invalid axis");
    }

    bool Unit::isTurnInProgress(const std::string& pieceName, Axis axis) const
    {
        auto piece = mesh.find(pieceName);
        if (!piece)
        {
            throw std::runtime_error("Invalid piece name " + pieceName);
        }

        switch (axis)
        {
            case Axis::X:
                return !!(piece->xTurnOperation);
            case Axis::Y:
                return !!(piece->yTurnOperation);
            case Axis::Z:
                return !!(piece->zTurnOperation);
        }

        throw std::logic_error("Invalid axis");
    }
}
