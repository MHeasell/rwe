#include "RenderBatch_helpers.h"
#include "MapTerrain.h"
#include "Unit.h"
#include "matrix_util.h"
#include <rwe/math/Vector3f.h>
#include <rwe/math/Matrix4f.h>

namespace rwe
{
    void createShadowRenderCommand(const MapTerrain& terrain, const Unit& unit, const Matrix4f& viewProjectionMatrix, std::vector<MeshShadowRenderCommand>& commands)
    {
        auto groundHeight = terrain.getHeightAt(unit.position.x, unit.position.z);
        auto shadowProjectionMatrix = Matrix4f::translation(Vector3f(0.0f, groundHeight, 0.0f))
            * Matrix4f::scale(Vector3f(1.0f, 0.0f, 1.0f))
            * Matrix4f::shearXZ(0.25f, -0.25f)
            * Matrix4f::translation(Vector3f(0.0f, -groundHeight, 0.0f));
        auto modelMatrix = Matrix4f::translation(simVectorToFloat(unit.position)) * Matrix4f::rotationY(toRadians(unit.rotation).value);

        auto mvpMatrix = viewProjectionMatrix * shadowProjectionMatrix * modelMatrix;

        createShadowRenderCommand(terrain, unit.mesh, mvpMatrix, commands);
    }

    void createShadowRenderCommand(const MapTerrain& terrain, const UnitMesh& mesh, const Matrix4f& parentMvpMatrix, std::vector<MeshShadowRenderCommand>& commands)
    {
        if (!mesh.visible)
        {
            return;
        }

        auto mvpMatrix = parentMvpMatrix * toFloatMatrix(mesh.getTransform());
        commands.emplace_back(mvpMatrix, &mesh.mesh->texturedVertices);

        for (const auto& c : mesh.children)
        {
            createShadowRenderCommand(terrain, mesh, mvpMatrix, commands);
        }
    }

    SpriteRenderCommand createRenderCommand(const MapFeature& feature, const Matrix4f& viewProjectionMatrix)
    {
        auto position = simVectorToFloat(feature.position);
        const auto& sprite = *feature.animation->sprites[0];

        float alpha = feature.transparentAnimation ? 0.5f : 1.0f;

        Vector3f snappedPosition(std::round(position.x), truncateToInterval(position.y, 2.0f), std::round(position.z));

        // Convert to a model position that makes sense in the game world.
        // For standing (blocking) features we stretch y-dimension values by 2x
        // to correct for TA camera distortion.
        auto conversionMatrix = feature.isStanding()
            ? Matrix4f::scale(Vector3f(1.0f, -2.0f, 1.0f))
            : Matrix4f::rotationX(-Pif / 2.0f) * Matrix4f::scale(Vector3f(1.0f, -1.0f, 1.0f));

        auto modelMatrix = Matrix4f::translation(snappedPosition) * conversionMatrix * sprite.getTransform();
        auto mvpMatrix = viewProjectionMatrix * modelMatrix;

        return SpriteRenderCommand{sprite.texture.get(), mvpMatrix, sprite.mesh.get(), alpha};
    }
}
