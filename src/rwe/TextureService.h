#ifndef RWE_TEXTURESERVICE_H
#define RWE_TEXTURESERVICE_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SharedTextureHandle.h>
#include <rwe/SpriteSeries.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <rwe/ColorPalette.h>
#include <unordered_map>

namespace rwe
{
    class TextureService
    {
    private:
        struct TextureInfo
        {
            unsigned int width;
            unsigned int height;
            SharedTextureHandle handle;

            TextureInfo() = default;
            TextureInfo(unsigned int width, unsigned int height, const SharedTextureHandle& handle);
        };

        GraphicsContext* graphics;
        AbstractVirtualFileSystem* fileSystem;
        const ColorPalette* palette;

        std::shared_ptr<SpriteSeries> defaultSpriteSeries;

        std::unordered_map<std::string, std::shared_ptr<SpriteSeries>> animCache;
        std::unordered_map<std::string, TextureInfo> bitmapCache;

    public:
        TextureService(GraphicsContext* graphics, AbstractVirtualFileSystem* filesystem, const ColorPalette* palette);

        std::shared_ptr<SpriteSeries> getGafEntry(const std::string& gafName, const std::string& entryName);
        boost::optional<std::shared_ptr<SpriteSeries>> getGuiTexture(const std::string& guiName, const std::string& graphicName);
        SharedTextureHandle getBitmap(const std::string& bitmapName);
        Sprite getBitmapRegion(const std::string& bitmapName, int x, int y, int width, int height);
        SharedTextureHandle getDefaultTexture();
        std::shared_ptr<SpriteSeries> getDefaultSpriteSeries();

    private:
        boost::optional<std::shared_ptr<SpriteSeries>> getGafEntryInternal(const std::string& gafName, const std::string& entryName);
        TextureInfo getBitmapInternal(const std::string& bitmapName);
    };
}

#endif
