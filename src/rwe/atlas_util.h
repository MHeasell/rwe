#pragma once

#include <rwe/ColorPalette.h>
#include <rwe/GraphicsContext.h>
#include <rwe/TextureHandle.h>
#include <rwe/geometry/Rectangle2f.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <string>
#include <unordered_map>
#include <vector>


namespace rwe
{
    struct TextureAtlasInfo
    {
        SharedTextureHandle textureAtlas;
        std::unordered_map<std::string, Rectangle2f> textureAtlasMap;
        std::vector<Vector2f> colorAtlasMap;

        std::vector<SharedTextureHandle> teamTextureAtlases;
        std::unordered_map<std::string, Rectangle2f> teamTextureAtlasMap;
    };

    TextureAtlasInfo createTextureAtlases(AbstractVirtualFileSystem* vfs, GraphicsContext* graphics, const ColorPalette* palette);
}
