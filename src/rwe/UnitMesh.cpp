#include "UnitMesh.h"
#include <rwe/float_math.h>

#include <rwe/math/Matrix4f.h>
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

namespace rwe
{
    void applyMoveOperation(std::optional<UnitMesh::MoveOperation>& op, SimScalar& currentPos, SimScalar dt)
    {
        if (op)
        {
            auto remaining = op->targetPosition - currentPos;
            auto frameSpeed = op->speed * dt;
            if (abs(remaining) <= frameSpeed)
            {
                currentPos = op->targetPosition;
                op = std::nullopt;
            }
            else
            {
                currentPos += frameSpeed * (remaining > 0_ss ? 1_ss : -1_ss);
            }
        }
    }

    void applyTurnOperation(std::optional<UnitMesh::TurnOperationUnion>& op, SimAngle& currentAngle, SimScalar dt)
    {
        if (!op)
        {
            return;
        }

        if (auto turnOp = std::get_if<UnitMesh::TurnOperation>(&*op); turnOp != nullptr)
        {
            auto frameSpeed = simAngleFromSimScalar(turnOp->speed * dt);
            currentAngle = turnTowards(currentAngle, turnOp->targetAngle, frameSpeed);
            if (currentAngle == turnOp->targetAngle)
            {
                op = std::nullopt;
            }

            return;
        }

        if (auto spinOp = std::get_if<UnitMesh::SpinOperation>(&*op); spinOp != nullptr)
        {
            auto frameAccel = spinOp->acceleration / 2_ss;
            auto remaining = spinOp->targetSpeed - spinOp->currentSpeed;
            if (abs(remaining) <= frameAccel)
            {
                spinOp->currentSpeed = spinOp->targetSpeed;
            }
            else
            {
                spinOp->currentSpeed += frameAccel * (remaining > 0_ss ? 1_ss : -1_ss);
            }

            auto frameSpeed = spinOp->currentSpeed * dt;
            if (frameSpeed > 0_ss)
            {
                currentAngle += SimAngle(frameSpeed.value);
            }
            else
            {
                currentAngle -= SimAngle(-frameSpeed.value);
            }
            return;
        }

        if (auto stopSpinOp = std::get_if<UnitMesh::StopSpinOperation>(&*op); stopSpinOp != nullptr)
        {
            auto frameDecel = stopSpinOp->deceleration / 2_ss;
            if (abs(stopSpinOp->currentSpeed) <= frameDecel)
            {
                op = std::nullopt;
                return;
            }

            stopSpinOp->currentSpeed -= frameDecel * (stopSpinOp->currentSpeed > 0_ss ? 1_ss : -1_ss);
            auto frameSpeed = stopSpinOp->currentSpeed * dt;
            if (frameSpeed > 0_ss)
            {
                currentAngle += SimAngle(frameSpeed.value);
            }
            else
            {
                currentAngle -= SimAngle(-frameSpeed.value);
            }
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

    std::optional<Matrix4x<SimScalar>> UnitMesh::getPieceTransform(const std::string& pieceName) const
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

    Matrix4x<SimScalar> UnitMesh::getTransform() const
    {
        return Matrix4x<SimScalar>::translation(origin + offset)
            * Matrix4x<SimScalar>::rotationZXY(
                sin(rotationX),
                cos(rotationX),
                sin(rotationY),
                cos(rotationY),
                sin(rotationZ),
                cos(rotationZ));
    }

    void UnitMesh::update(SimScalar dt)
    {
        applyMoveOperation(xMoveOperation, offset.x, dt);
        applyMoveOperation(yMoveOperation, offset.y, dt);
        applyMoveOperation(zMoveOperation, offset.z, dt);

        applyTurnOperation(xTurnOperation, rotationX, dt);
        applyTurnOperation(yTurnOperation, rotationY, dt);
        applyTurnOperation(zTurnOperation, rotationZ, dt);

        for (auto& c : children)
        {
            c.update(dt);
        }
    }

    UnitMesh::MoveOperation::MoveOperation(SimScalar targetPosition, SimScalar speed)
        : targetPosition(targetPosition), speed(speed)
    {
    }

    UnitMesh::TurnOperation::TurnOperation(SimAngle targetAngle, SimScalar speed)
        : targetAngle(targetAngle), speed(speed)
    {
    }
}
