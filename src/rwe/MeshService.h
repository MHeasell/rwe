#ifndef RWE_MESHSERVICE_H
#define RWE_MESHSERVICE_H

#include <memory>
#include <rwe/TextureService.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include "UnitMesh.h"
#include "_3do.h"
#include <boost/functional/hash.hpp>

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
    private:
        AbstractVirtualFileSystem* vfs;
        GraphicsContext* graphics;
        const ColorPalette* palette;
        SharedTextureHandle atlas;
        std::unordered_map<FrameId, Rectangle2f> atlasMap;

    public:
        static MeshService createMeshService(
            AbstractVirtualFileSystem* vfs,
            GraphicsContext* graphics,
            const ColorPalette* palette);

        MeshService(
            AbstractVirtualFileSystem* vfs,
            const ColorPalette* palette,
            SharedTextureHandle&& atlas,
            std::unordered_map<FrameId, Rectangle2f>&& atlasMap);

        struct UnitMeshInfo
        {
            UnitMesh mesh;
            SelectionMesh selectionMesh;
        };

        UnitMeshInfo loadUnitMesh(const std::string& name, unsigned int playerId);

    private:
        SharedTextureHandle getMeshTextureAtlas();
        Rectangle2f getTextureRegion(const std::string& name, unsigned int frameNumber);

        Mesh meshFrom3do(const _3do::Object& o, unsigned int playerId);

        UnitMesh unitMeshFrom3do(const _3do::Object& o, unsigned int playerId);

        SelectionMesh selectionMeshFrom3do(const _3do::Object& o);
    };
}


#endif
