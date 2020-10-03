#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/PlayerColorIndex.h>
#include <rwe/SelectionMesh.h>
#include <rwe/TextureService.h>
#include <rwe/UnitModelDefinition.h>
#include <rwe/io/_3do/_3do.h>
#include <rwe/sim/UnitMesh.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>


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
            std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>> pieceMeshes;
            SelectionMesh selectionMesh;
        };

        struct ProjectileMeshInfo
        {
            UnitModelDefinition modelDefinition;
            std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>> pieceMeshes;
        };

        UnitMeshInfo loadUnitMesh(const std::string& name);
        ProjectileMeshInfo loadProjectileMesh(const std::string& name);

    private:
        struct TextureRegionInfo
        {
            bool isTeamColor;
            Rectangle2f region;
        };
        TextureRegionInfo getTextureRegion(const std::string& name);
        Vector2f getColorTexturePoint(unsigned int colorIndex);

        void extractMeshes(const _3do::Object& o, std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>>& v);

        Mesh meshFrom3do(const _3do::Object& o);

        SelectionMesh selectionMeshFrom3do(const _3do::Object& o);

        GlMesh createSelectionMesh(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d);

        ShaderMesh convertMesh(const Mesh& mesh);
    };
}
