#include "MeshService.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/io/_3do/_3do.h>
#include <rwe/math/Vector2f.h>
#include <rwe/mesh_util.h>
#include <rwe/util/rwe_string.h>
#include <rwe/vertex_height.h>

namespace rwe
{
    MeshService::MeshService(
        AbstractVirtualFileSystem* vfs,
        GraphicsContext* graphics,
        std::unordered_map<std::string, Rectangle2f>&& atlasMap,
        std::unordered_map<std::string, Rectangle2f>&& teamAtlasMap,
        std::vector<Vector2f>&& atlasColorMap)
        : vfs(vfs),
          graphics(graphics),
          atlasMap(std::move(atlasMap)),
          teamAtlasMap(std::move(teamAtlasMap)),
          atlasColorMap(std::move(atlasColorMap))
    {
    }

    MeshService::UnitMeshInfo MeshService::loadUnitMesh(const std::string& name)
    {
        auto bytes = vfs->readFile("objects3d/" + name + ".3do");
        if (!bytes)
        {
            throw std::runtime_error("Failed to load object bytes: " + name);
        }

        boost::interprocess::bufferstream s(bytes->data(), bytes->size());
        auto objects = parse3doObjects(s, s.tellg());
        assert(objects.size() == 1);
        auto selectionMesh = selectionMeshFrom3do(*graphics, objects.front());

        auto d = createUnitModelDefinition(
            simScalarFromFixed(findHighestVertex(objects.front()).y),
            unitMeshFrom3do(objects.front()));

        std::vector<std::pair<std::string, UnitPieceMeshInfo>> meshes;
        extractMeshes(*graphics, atlasMap, teamAtlasMap, atlasColorMap, objects.front(), meshes);

        return UnitMeshInfo{std::move(d), std::move(meshes), std::move(selectionMesh)};
    }

    MeshService::ProjectileMeshInfo MeshService::loadProjectileMesh(const std::string& name)
    {
        auto bytes = vfs->readFile("objects3d/" + name + ".3do");
        if (!bytes)
        {
            throw std::runtime_error("Failed to load object bytes: " + name);
        }

        boost::interprocess::bufferstream s(bytes->data(), bytes->size());
        auto objects = parse3doObjects(s, s.tellg());
        assert(objects.size() == 1);

        auto d = createUnitModelDefinition(
            simScalarFromFixed(findHighestVertex(objects.front()).y),
            unitMeshFrom3do(objects.front()));

        std::vector<std::pair<std::string, UnitPieceMeshInfo>> meshes;
        extractMeshes(*graphics, atlasMap, teamAtlasMap, atlasColorMap, objects.front(), meshes);

        return ProjectileMeshInfo{std::move(d), std::move(meshes)};
    }
}
