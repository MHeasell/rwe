#ifndef RWE_UNITMESH_H
#define RWE_UNITMESH_H

#include <memory>
#include <optional>
#include <rwe/RadiansAngle.h>
#include <rwe/ShaderMesh.h>
#include <rwe/math/Matrix4f.h>
#include <rwe/math/Vector3f.h>
#include <string>
#include <vector>


namespace rwe
{
    struct UnitMesh
    {
        struct MoveOperation
        {
            float targetPosition;
            float speed;

            MoveOperation(float targetPosition, float speed);
        };

        struct TurnOperation
        {
            RadiansAngle targetAngle;
            float speed;

            TurnOperation(RadiansAngle targetAngle, float speed);
        };

        std::string name;
        Vector3f origin;
        std::shared_ptr<ShaderMesh> mesh;
        std::vector<UnitMesh> children;
        bool visible{true};
        Vector3f offset{0.0f, 0.0f, 0.0f};
        Vector3f rotation{0.0f, 0.0f, 0.0f};

        std::optional<MoveOperation> xMoveOperation;
        std::optional<MoveOperation> yMoveOperation;
        std::optional<MoveOperation> zMoveOperation;

        std::optional<TurnOperation> xTurnOperation;
        std::optional<TurnOperation> yTurnOperation;
        std::optional<TurnOperation> zTurnOperation;

        std::optional<std::reference_wrapper<const UnitMesh>> find(const std::string& pieceName) const;

        std::optional<std::reference_wrapper<UnitMesh>> find(const std::string& pieceName);

        std::optional<Matrix4f> getPieceTransform(const std::string& pieceName);

        Matrix4f getTransform() const;

        void update(float dt);
    };
}

#endif
