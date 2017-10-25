#ifndef RWE_UNITMESH_H
#define RWE_UNITMESH_H

#include <rwe/GraphicsContext.h>
#include <rwe/math/Vector3f.h>
#include <string>


namespace rwe
{
    struct UnitMesh
    {
        std::string name;
        Vector3f origin;
        std::shared_ptr<ShaderMesh> mesh;
        std::vector<UnitMesh> children;
        bool visible{true};

        void render(
            GraphicsContext& context,
            ShaderProgramIdentifier textureShader,
            ShaderProgramIdentifier colorShader,
            const Matrix4f& modelMatrix,
            const Matrix4f& viewMatrix,
            const Matrix4f& projectionMatrix) const;

        boost::optional<UnitMesh&> find(const std::string& pieceName);
    };
}

#endif
