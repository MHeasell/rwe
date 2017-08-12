#ifndef RWE_TEXTURESERVICE_H
#define RWE_TEXTURESERVICE_H

#include <memory>
#include <rwe/GraphicsContext.h>
#include <rwe/SharedTextureHandle.h>
#include <rwe/SpriteSeries.h>

namespace rwe
{
    class TextureService
    {
    public:
        std::shared_ptr<SpriteSeries> getGafEntry(const std::string& gafName, const std::string& entryName);
        std::shared_ptr<SpriteSeries> getGuiTexture(const std::string& guiName, const std::string& graphicName);
        SharedTextureHandle getBitmap(const std::string& bitmapName);
    };
}

#endif
