#ifndef RWE_TEXTURESERVICE_H
#define RWE_TEXTURESERVICE_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/ColorPalette.h>
#include <rwe/GraphicsContext.h>
#include <rwe/SpriteSeries.h>
#include <rwe/TextureHandle.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
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
        std::unordered_map<std::string, std::shared_ptr<Sprite>> minimapCache;

    public:
        TextureService(GraphicsContext* graphics, AbstractVirtualFileSystem* filesystem, const ColorPalette* palette);

        boost::optional<std::shared_ptr<SpriteSeries>> tryGetGafEntry(const std::string& gafName, const std::string& entryName);
        std::shared_ptr<SpriteSeries> getGafEntry(const std::string& gafName, const std::string& entryName);
        boost::optional<std::shared_ptr<SpriteSeries>> getGuiTexture(const std::string& guiName, const std::string& graphicName);
        SharedTextureHandle getBitmap(const std::string& bitmapName);
        std::shared_ptr<Sprite> getBitmapRegion(const std::string& bitmapName, int x, int y, int width, int height);
        SharedTextureHandle getDefaultTexture();
        std::shared_ptr<SpriteSeries> getDefaultSpriteSeries();
        std::shared_ptr<Sprite> getDefaultSprite();
        std::shared_ptr<Sprite> getMinimap(const std::string& mapName);

    private:
        boost::optional<std::shared_ptr<SpriteSeries>> getGafEntryInternal(const std::string& gafName, const std::string& entryName);
        TextureInfo getBitmapInternal(const std::string& bitmapName);
    };
}

#endif
