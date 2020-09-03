#pragma once

#include <boost/functional/hash.hpp>
#include <memory>
#include <rwe/PlayerColorIndex.h>
#include <rwe/TextureService.h>
#include <rwe/UnitMesh.h>
#include <rwe/_3do.h>
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
            UnitMesh mesh;
            SelectionMesh selectionMesh;
            SimScalar height;
        };

        UnitMeshInfo loadUnitMesh(const std::string& name);

        UnitMesh loadProjectileMesh(const std::string& name);

    private:
        struct TextureRegionInfo
        {
            bool isTeamColor;
            Rectangle2f region;
        };
        TextureRegionInfo getTextureRegion(const std::string& name);
        Vector2f getColorTexturePoint(unsigned int colorIndex);

        Mesh meshFrom3do(const _3do::Object& o);

        UnitMesh unitMeshFrom3do(const _3do::Object& o);

        SelectionMesh selectionMeshFrom3do(const _3do::Object& o);

        GlMesh createSelectionMesh(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d);

        ShaderMesh convertMesh(const Mesh& mesh);
    };
}
