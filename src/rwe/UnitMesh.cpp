#include "UnitMesh.h"
#include <rwe/math/Matrix4f.h>
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

namespace rwe
{
    void applyMoveOperation(std::optional<UnitMesh::MoveOperation>& op, float& currentPos, float dt)
    {
        if (op)
        {
            float remaining = op->targetPosition - currentPos;
            float frameSpeed = op->speed * dt;
            if (std::abs(remaining) <= frameSpeed)
            {
                currentPos = op->targetPosition;
                op = std::nullopt;
            }
            else
            {
                currentPos += frameSpeed * (remaining > 0.0f ? 1.0f : -1.0f);
            }
        }
    }

    void applyTurnOperation(std::optional<UnitMesh::TurnOperationUnion>& op, float& currentAngle, float dt)
    {
        if (!op)
        {
            return;
        }

        if (auto turnOp = std::get_if<UnitMesh::TurnOperation>(&*op); turnOp != nullptr)
        {
            auto remaining = turnOp->targetAngle - RadiansAngle(currentAngle);

            float frameSpeed = turnOp->speed * dt;
            if (std::abs(remaining.value) <= frameSpeed)
            {
                currentAngle = turnOp->targetAngle.value;
                op = std::nullopt;
                return;
            }

            auto angleDelta = frameSpeed * (remaining.value > 0.0f ? 1.0f : -1.0f);
            currentAngle = wrap(-Pif, Pif, currentAngle + angleDelta);
            return;
        }

        if (auto spinOp = std::get_if<UnitMesh::SpinOperation>(&*op); spinOp != nullptr)
        {
            auto frameAccel = spinOp->acceleration * dt;
            auto remaining = spinOp->targetSpeed - spinOp->currentSpeed;
            if (std::abs(remaining) <= frameAccel)
            {
                spinOp->currentSpeed = spinOp->targetSpeed;
            }
            else
            {
                spinOp->currentSpeed += frameAccel * (remaining > 0.0f ? 1.0f : -1.0f);
            }

            auto frameSpeed = spinOp->currentSpeed * dt;
            currentAngle = wrap(-Pif, Pif, currentAngle + frameSpeed);
            return;
        }

        if (auto stopSpinOp = std::get_if<UnitMesh::StopSpinOperation>(&*op); stopSpinOp != nullptr)
        {
            auto frameDecel = stopSpinOp->deceleration * dt;
            if (std::abs(stopSpinOp->currentSpeed) <= frameDecel)
            {
                op = std::nullopt;
                return;
            }

            stopSpinOp->currentSpeed -= frameDecel * (stopSpinOp->currentSpeed > 0.0f ? 1.0f : -1.0f);
            auto frameSpeed = stopSpinOp->currentSpeed * dt;
            currentAngle = wrap(-Pif, Pif, currentAngle + frameSpeed);
            return;
        }
    }

    std::optional<std::reference_wrapper<const UnitMesh>> UnitMesh::find(const std::string& pieceName) const
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

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<UnitMesh>> UnitMesh::find(const std::string& pieceName)
    {
        auto value = static_cast<const UnitMesh&>(*this).find(pieceName);
        if (!value)
        {
            return std::nullopt;
        }

        return std::ref(const_cast<UnitMesh&>(value->get()));
    }

    std::optional<Matrix4f> UnitMesh::getPieceTransform(const std::string& pieceName) const
    {
        if (pieceName == name)
        {
            return getTransform();
        }

        for (auto& c : children)
        {
            auto childTransform = c.getPieceTransform(pieceName);
            if (childTransform)
            {
                return getTransform() * (*childTransform);
            }
        }

        return std::nullopt;
    }

    Matrix4f UnitMesh::getTransform() const
    {
        Vector3f rotationVec(rotation.x, rotation.y, rotation.z);
        return Matrix4f::translation(origin) * Matrix4f::translation(offset) * Matrix4f::rotationZXY(rotationVec);
    }

    void UnitMesh::update(float dt)
    {
        applyMoveOperation(xMoveOperation, offset.x, dt);
        applyMoveOperation(yMoveOperation, offset.y, dt);
        applyMoveOperation(zMoveOperation, offset.z, dt);

        applyTurnOperation(xTurnOperation, rotation.x, dt);
        applyTurnOperation(yTurnOperation, rotation.y, dt);
        applyTurnOperation(zTurnOperation, rotation.z, dt);

        for (auto& c : children)
        {
            c.update(dt);
        }
    }

    UnitMesh::MoveOperation::MoveOperation(float targetPosition, float speed)
        : targetPosition(targetPosition), speed(speed)
    {
    }

    UnitMesh::TurnOperation::TurnOperation(RadiansAngle targetAngle, float speed)
        : targetAngle(targetAngle), speed(speed)
    {
    }
}
