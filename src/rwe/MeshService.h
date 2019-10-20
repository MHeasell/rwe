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
    using FrameId = std::pair<std::string, unsigned int>;
}

namespace std
{
    template <>
    struct hash<rwe::FrameId>
    {
        std::size_t operator()(const rwe::FrameId& f) const noexcept
        {
            return boost::hash<rwe::FrameId>()(f);
        }
    };
}

namespace rwe
{
    class MeshService
    {
    public:
        struct TextureAttributes
        {
            bool isTeamDependent;
        };

    private:
        AbstractVirtualFileSystem* vfs;
        GraphicsContext* graphics;
        const ColorPalette* palette;
        SharedTextureHandle atlas;
        std::unordered_map<FrameId, Rectangle2f> atlasMap;
        std::unordered_map<std::string, TextureAttributes> textureAttributesMap;
        std::vector<Vector2f> atlasColorMap;

    public:
        static MeshService createMeshService(
            AbstractVirtualFileSystem* vfs,
            GraphicsContext* graphics,
            const ColorPalette* palette);

        MeshService(
            AbstractVirtualFileSystem* vfs,
            const ColorPalette* palette,
            SharedTextureHandle&& atlas,
            std::unordered_map<FrameId, Rectangle2f>&& atlasMap,
            std::unordered_map<std::string, TextureAttributes> textureAttributesMap,
            std::vector<Vector2f>&& atlasColorMap);

        struct UnitMeshInfo
        {
            UnitMesh mesh;
            SelectionMesh selectionMesh;
            SimScalar height;
        };

        UnitMeshInfo loadUnitMesh(const std::string& name, const PlayerColorIndex& teamColor);

        UnitMesh loadProjectileMesh(const std::string& name, const PlayerColorIndex& teamColor);

    private:
        SharedTextureHandle getMeshTextureAtlas();
        Rectangle2f getTextureRegion(const std::string& name, const PlayerColorIndex& teamColor);
        Vector2f getColorTexturePoint(unsigned int colorIndex);

        Mesh meshFrom3do(const _3do::Object& o, const PlayerColorIndex& teamColor);

        UnitMesh unitMeshFrom3do(const _3do::Object& o, const PlayerColorIndex& teamColor);

        SelectionMesh selectionMeshFrom3do(const _3do::Object& o);

        GlMesh createSelectionMesh(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d);

        ShaderMesh convertMesh(const Mesh& mesh);
    };
}
