#pragma once

#include <rwe/SelectionMesh.h>
#include <rwe/TextureService.h>
#include <rwe/UnitModelDefinition.h>
#include <rwe/UnitPieceMeshInfo.h>
#include <rwe/io/_3do/_3do.h>
#include <rwe/math/Vector2f.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rwe
{
    class MeshService
    {
    private:
        AbstractVirtualFileSystem* vfs;
        GraphicsContext* graphics;
        std::unordered_map<std::string, Rectangle2f> atlasMap;
        std::unordered_map<std::string, Rectangle2f> teamAtlasMap;
        std::vector<Vector2f> atlasColorMap;

    public:
        MeshService(
            AbstractVirtualFileSystem* vfs,
            GraphicsContext* graphics,
            std::unordered_map<std::string, Rectangle2f>&& atlasMap,
            std::unordered_map<std::string, Rectangle2f>&& teamAtlasMap,
            std::vector<Vector2f>&& atlasColorMap);

        struct UnitMeshInfo
        {
            UnitModelDefinition modelDefinition;
            std::vector<std::pair<std::string, UnitPieceMeshInfo>> pieceMeshes;
            SelectionMesh selectionMesh;
        };

        UnitMeshInfo loadUnitMesh(const std::string& name);

        struct ProjectileMeshInfo
        {
            UnitModelDefinition modelDefinition;
            std::vector<std::pair<std::string, UnitPieceMeshInfo>> pieceMeshes;
        };

        ProjectileMeshInfo loadProjectileMesh(const std::string& name);
    };
}
