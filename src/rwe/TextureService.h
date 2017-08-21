#ifndef RWE_TEXTURESERVICE_H
#define RWE_TEXTURESERVICE_H

#include <boost/optional.hpp>
#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SharedTextureHandle.h>
#include <rwe/SpriteSeries.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <rwe/ColorPalette.h>

namespace rwe
{
    class TextureService
    {
    private:
        GraphicsContext* graphics;
        AbstractVirtualFileSystem* fileSystem;
        const ColorPalette* palette;

        std::shared_ptr<SpriteSeries> defaultSpriteSeries;

    public:
        TextureService(GraphicsContext* graphics, AbstractVirtualFileSystem* filesystem, const ColorPalette* palette);

        std::shared_ptr<SpriteSeries> getGafEntry(const std::string& gafName, const std::string& entryName);
        boost::optional<std::shared_ptr<SpriteSeries>> getGuiTexture(const std::string& guiName, const std::string& graphicName);
        SharedTextureHandle getBitmap(const std::string& bitmapName);
        SharedTextureHandle getDefaultTexture();
        std::shared_ptr<SpriteSeries> getDefaultSpriteSeries();

    private:
        boost::optional<std::shared_ptr<SpriteSeries>> getGafEntryInternal(const std::string& gafName, const std::string& entryName);

    };
}

#endif
