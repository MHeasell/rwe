#ifndef RWE_UNIT_H
#define RWE_UNIT_H

#include <rwe/UnitMesh.h>
#include "GraphicsContext.h"
#include <memory>
#include <rwe/cob/CobEnvironment.h>
#include <boost/optional.hpp>
#include <rwe/geometry/BoundingBox3f.h>
#include <rwe/geometry/CollisionMesh.h>

namespace rwe
{
    class Unit
    {
    public:
        UnitMesh mesh;
        Vector3f position;
        std::unique_ptr<CobEnvironment> cobEnvironment;
        CollisionMesh selectionMesh;

        Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment, const CollisionMesh& selcetionMesh);

        void moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed);

        void moveObjectNow(const std::string& pieceName, Axis axis, float targetPosition);

        void turnObject(const std::string& pieceName, Axis axis, float targetAngle, float speed);

        void turnObjectNow(const std::string& pieceName, Axis axis, float targetAngle);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        bool isTurnInProgress(const std::string& pieceName, Axis axis) const;

        /**
         * Returns a value if the given ray intersects this unit
         * for the purposes of unit selection.
         * The value returned is the distance along the ray
         * where the intersection occurred.
         */
        boost::optional<float> selectionIntersect(const Ray3f& ray) const;

        void render(
            GraphicsContext& context,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;

        void renderSelectionRect(
            GraphicsContext& context,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;
    };
}

#endif
