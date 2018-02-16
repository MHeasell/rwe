#ifndef RWE_UNITMESH_H
#define RWE_UNITMESH_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/RadiansAngle.h>
#include <rwe/ShaderMesh.h>
#include <rwe/math/Vector3f.h>
#include <string>


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

        boost::optional<MoveOperation> xMoveOperation;
        boost::optional<MoveOperation> yMoveOperation;
        boost::optional<MoveOperation> zMoveOperation;

        boost::optional<TurnOperation> xTurnOperation;
        boost::optional<TurnOperation> yTurnOperation;
        boost::optional<TurnOperation> zTurnOperation;

        boost::optional<const UnitMesh&> find(const std::string& pieceName) const;

        boost::optional<UnitMesh&> find(const std::string& pieceName);

        void update(float dt);
    };
}

#endif
