#pragma once

#include <optional>
#include <rwe/sim/SimAngle.h>
#include <rwe/sim/SimScalar.h>
#include <rwe/sim/SimVector.h>
#include <variant>


namespace rwe
{
    struct UnitMesh
    {
        struct MoveOperation
        {
            SimScalar targetPosition;
            SimScalar speed;

            MoveOperation(SimScalar targetPosition, SimScalar speed);
        };

        struct TurnOperation
        {
            SimAngle targetAngle;
            SimScalar speed;

            TurnOperation(SimAngle targetAngle, SimScalar speed);
        };

        struct SpinOperation
        {
            SimScalar currentSpeed;
            SimScalar targetSpeed;
            SimScalar acceleration;

            SpinOperation(SimScalar currentSpeed, SimScalar targetSpeed, SimScalar acceleration)
                : currentSpeed(currentSpeed),
                  targetSpeed(targetSpeed),
                  acceleration(acceleration)
            {
            }
        };

        struct StopSpinOperation
        {
            SimScalar currentSpeed;
            SimScalar deceleration;

            StopSpinOperation(SimScalar currentSpeed, SimScalar deceleration)
                : currentSpeed(currentSpeed),
                  deceleration(deceleration)
            {
            }
        };

        using TurnOperationUnion = std::variant<TurnOperation, SpinOperation, StopSpinOperation>;

        std::string name;
        bool visible{true};
        bool shaded{true};
        SimVector offset{0_ss, 0_ss, 0_ss};
        SimVector previousOffset{0_ss, 0_ss, 0_ss};
        SimAngle previousRotationX{0};
        SimAngle previousRotationY{0};
        SimAngle previousRotationZ{0};
        SimAngle rotationX{0};
        SimAngle rotationY{0};
        SimAngle rotationZ{0};

        std::optional<MoveOperation> xMoveOperation;
        std::optional<MoveOperation> yMoveOperation;
        std::optional<MoveOperation> zMoveOperation;

        std::optional<TurnOperationUnion> xTurnOperation;
        std::optional<TurnOperationUnion> yTurnOperation;
        std::optional<TurnOperationUnion> zTurnOperation;

        void update(SimScalar dt);
    };
}
