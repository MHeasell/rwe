#pragma once

#include <memory>
#include <optional>
#include <rwe/RadiansAngle.h>
#include <rwe/ShaderMesh.h>
#include <rwe/SimAngle.h>
#include <rwe/math/Matrix4f.h>
#include <rwe/math/Vector3f.h>
#include <string>
#include <variant>
#include <vector>


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
        Vector3x<SimScalar> origin;
        std::vector<UnitMesh> children;
        bool visible{true};
        bool shaded{true};
        Vector3x<SimScalar> offset{0_ss, 0_ss, 0_ss};
        SimAngle rotationX{0};
        SimAngle rotationY{0};
        SimAngle rotationZ{0};

        std::optional<MoveOperation> xMoveOperation;
        std::optional<MoveOperation> yMoveOperation;
        std::optional<MoveOperation> zMoveOperation;

        std::optional<TurnOperationUnion> xTurnOperation;
        std::optional<TurnOperationUnion> yTurnOperation;
        std::optional<TurnOperationUnion> zTurnOperation;

        std::optional<std::reference_wrapper<const UnitMesh>> find(const std::string& pieceName) const;

        std::optional<std::reference_wrapper<UnitMesh>> find(const std::string& pieceName);

        std::optional<Matrix4x<SimScalar>> getPieceTransform(const std::string& pieceName) const;

        Matrix4x<SimScalar> getTransform() const;

        void update(SimScalar dt);
    };
}
