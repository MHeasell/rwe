#ifndef RWE_UNIT_H
#define RWE_UNIT_H

#include <rwe/UnitMesh.h>
#include "GraphicsContext.h"
#include <memory>
#include <rwe/cob/CobEnvironment.h>
#include <boost/optional.hpp>

namespace rwe
{
    class Unit
    {
    public:
        UnitMesh mesh;
        Vector3f position;
        std::unique_ptr<CobEnvironment> cobEnvironment;

        Unit(const UnitMesh& mesh, std::unique_ptr<CobEnvironment>&& cobEnvironment);

        void moveObject(const std::string& pieceName, Axis axis, float targetPosition, float speed);

        bool isMoveInProgress(const std::string& pieceName, Axis axis) const;

        void render(
            GraphicsContext& context,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;
    };
}

#endif
