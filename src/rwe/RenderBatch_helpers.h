#pragma once

#include "MapFeature.h"
#include "MapTerrain.h"
#include "RenderBatch.h"
#include "Unit.h"

namespace rwe
{
    void createShadowRenderCommand(const MapTerrain& terrain, const Unit& unit, const Matrix4f& viewProjectionMatrix, std::vector<MeshShadowRenderCommand>& commands);
    void createShadowRenderCommand(const MapTerrain& terrain, const UnitMesh& mesh, const Matrix4f& parentMvpMatrix, std::vector<MeshShadowRenderCommand>& commands);
    SpriteRenderCommand createRenderCommand(const MapFeature& feature, const Matrix4f& viewProjectionMatrix);
}
