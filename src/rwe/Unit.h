#ifndef RWE_UNIT_H
#define RWE_UNIT_H

#include <rwe/UnitMesh.h>
#include "GraphicsContext.h"

namespace rwe
{
    class Unit
    {
    public:
        UnitMesh mesh;
        Vector3f position;

        void render(
            GraphicsContext& context,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;
    };
}

#endif
