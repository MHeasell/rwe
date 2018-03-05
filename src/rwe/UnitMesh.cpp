#include "UnitMesh.h"
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>

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

    void applyTurnOperation(boost::optional<UnitMesh::TurnOperation>& op, float& currentAngle, float dt)
    {
        if (op)
        {
            auto remaining = op->targetAngle - RadiansAngle(currentAngle);

            float frameSpeed = op->speed * dt;
            if (std::abs(remaining.value) <= frameSpeed)
            {
                currentAngle = op->targetAngle.value;
                op = boost::none;
            }
            else
            {
                auto angleDelta = frameSpeed * (remaining.value > 0.0f ? 1.0f : -1.0f);
                currentAngle = wrap(-Pif, Pif, currentAngle + angleDelta);
            }
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
