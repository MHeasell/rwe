#include "UnitMesh.h"

#include <rwe/math/rwe_math.h>

namespace rwe
{
    void applyMoveOperation(std::optional<UnitMesh::MoveOperation>& op, SimScalar& currentPos, SimScalar dt)
    {
        if (op)
        {
            auto remaining = op->targetPosition - currentPos;
            auto frameSpeed = op->speed * dt;
            if (rweAbs(remaining) <= frameSpeed)
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
            auto frameAccel = spinOp->acceleration;
            auto remaining = spinOp->targetSpeed - spinOp->currentSpeed;
            if (rweAbs(remaining) <= frameAccel)
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
            auto frameDecel = stopSpinOp->deceleration;
            if (rweAbs(stopSpinOp->currentSpeed) <= frameDecel)
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

    void UnitMesh::update(SimScalar dt)
    {
        previousOffset = offset;
        applyMoveOperation(xMoveOperation, offset.x, dt);
        applyMoveOperation(yMoveOperation, offset.y, dt);
        applyMoveOperation(zMoveOperation, offset.z, dt);

        previousRotationX = rotationX;
        previousRotationY = rotationY;
        previousRotationZ = rotationZ;
        applyTurnOperation(xTurnOperation, rotationX, dt);
        applyTurnOperation(yTurnOperation, rotationY, dt);
        applyTurnOperation(zTurnOperation, rotationZ, dt);
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
