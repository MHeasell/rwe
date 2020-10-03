#include "TextureService.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/io/fnt/Fnt.h>
#include <rwe/io/gaf/Gaf.h>
#include <rwe/io/tnt/TntArchive.h>
#include <rwe/pcx.h>
#include <rwe/rwe_string.h>

namespace rwe
{
    class BufferGafAdapter : public GafReaderAdapter
    {
    private:
        GraphicsContext* graphics;
        const ColorPalette* palette;
        std::vector<Color> buffer;
        GafFrameData currentFrameHeader;

        SpriteSeries spriteSeries;

    public:
        explicit BufferGafAdapter(GraphicsContext* graphics, const ColorPalette* palette) : graphics(graphics), palette(palette), currentFrameHeader() {}

        void beginFrame(const GafFrameData& header) override
        {
            buffer.clear();
            buffer.resize(header.width * header.height);
            std::fill(buffer.begin(), buffer.end(), Color::Transparent);
            currentFrameHeader = header;
        }

        void frameLayer(const LayerData& data) override
        {
            for (std::size_t y = 0; y < data.height; ++y)
            {
                for (std::size_t x = 0; x < data.width; ++x)
                {
                    auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                    auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                    if (outPosX < 0 || outPosX >= currentFrameHeader.width || outPosY < 0 || outPosY >= currentFrameHeader.height)
                    {
                        // Some third-party gafs (e.g. "FAVBOOM.gaf" for the CORMKL [Cybran Monkeylord] unit)
                        // contain layers whose bounds exceed the dimensions of the frame.
                        // If this happens we'll just ignore the pixels that are out of bounds.
                        continue;
                    }

                    auto colorIndex = static_cast<unsigned char>(data.data[(y * data.width) + x]);
                    if (colorIndex == data.transparencyKey)
                    {
                        continue;
                    }

                    buffer[(outPosY * currentFrameHeader.width) + outPosX] = (*palette)[colorIndex];
                }
            }
        }

        void endFrame() override
        {
            SharedTextureHandle handle(graphics->createTexture(currentFrameHeader.width, currentFrameHeader.height, buffer));

            auto bounds = Rectangle2f::fromTopLeft(
                -currentFrameHeader.posX,
                -currentFrameHeader.posY,
                currentFrameHeader.width,
                currentFrameHeader.height);

            auto region = Rectangle2f::fromTopLeft(0.0f, 0.0f, 1.0f, 1.0f);

            auto sprite = std::make_shared<Sprite>(graphics->createSprite(bounds, region, handle));
            spriteSeries.sprites.push_back(std::move(sprite));
        }

        SpriteSeries extractSpriteSeries()
        {
            return std::move(spriteSeries);
        }
    };

    TextureService::TextureService(GraphicsContext* graphics, AbstractVirtualFileSystem* fileSystem, const ColorPalette* palette)
        : graphics(graphics), fileSystem(fileSystem), palette(palette)
    {
        SharedTextureHandle handle(graphics->createColorTexture(Color(255, 0, 255)));
        auto sprite = graphics->createSprite(
            Rectangle2f(0.5f, 0.5f, 0.5f, 0.5f),
            Rectangle2f(0.5f, 0.5f, 0.5f, 0.5f),
            handle);

        auto series = std::make_shared<SpriteSeries>();
        series->sprites.push_back(std::make_shared<Sprite>(std::move(sprite)));
        defaultSpriteSeries = std::move(series);
    }

    std::optional<std::shared_ptr<SpriteSeries>> TextureService::getGafEntryInternal(const std::string& gafName, const std::string& entryName)
    {
        auto normEntryName = toUpper(entryName);

        auto key = gafName + "/" + normEntryName;
        auto it = animCache.find(key);
        if (it != animCache.end())
        {
            return it->second;
        }

        auto gafBytes = fileSystem->readFile(gafName);
        if (!gafBytes)
        {
            return std::nullopt;
        }

        boost::interprocess::bufferstream gafStream(gafBytes->data(), gafBytes->size());
        GafArchive gafArchive(&gafStream);

        auto gafEntry = gafArchive.findEntry(normEntryName);
        if (!gafEntry)
        {
            return std::nullopt;
        }

        BufferGafAdapter adapter(graphics, palette);
        gafArchive.extract(*gafEntry, adapter);
        auto ptr = std::make_shared<SpriteSeries>(adapter.extractSpriteSeries());
        animCache[key] = ptr;
        return ptr;
    }

    std::optional<std::shared_ptr<SpriteSeries>>
    TextureService::tryGetGafEntry(const std::string& gafName, const std::string& entryName)
    {
        return getGafEntryInternal(gafName, entryName);
    }

    std::shared_ptr<SpriteSeries> TextureService::getGafEntry(const std::string& gafName, const std::string& entryName)
    {
        auto entry = getGafEntryInternal(gafName, entryName);
        if (!entry)
        {
            throw std::runtime_error("Failed to load GAF entry");
        }

        return *entry;
    }

    std::optional<std::shared_ptr<SpriteSeries>>
    TextureService::getGuiTexture(const std::string& guiName, const std::string& graphicName)
    {
        auto entry = getGafEntryInternal("anims/" + guiName + ".gaf", graphicName);
        if (entry)
        {
            return entry;
        }

        entry = getGafEntryInternal("anims/commongui.gaf", graphicName);
        if (entry)
        {
            return entry;
        }

        return std::nullopt;
    }

    SharedTextureHandle TextureService::getBitmap(const std::string& bitmapName)
    {
        auto info = getBitmapInternal(bitmapName);
        return info.handle;
    }

    std::shared_ptr<SpriteSeries> TextureService::getDefaultSpriteSeries()
    {
        return defaultSpriteSeries;
    }

    SharedTextureHandle TextureService::getDefaultTexture()
    {
        return defaultSpriteSeries->sprites[0]->texture;
    }

    std::shared_ptr<Sprite> TextureService::getBitmapRegion(const std::string& bitmapName, int x, int y, int width, int height)
    {
        auto bitmap = getBitmapInternal(bitmapName);
        auto region = Rectangle2f::fromTopLeft(
            static_cast<float>(x) / static_cast<float>(bitmap.width),
            static_cast<float>(y) / static_cast<float>(bitmap.height),
            static_cast<float>(width) / static_cast<float>(bitmap.width),
            static_cast<float>(height) / static_cast<float>(bitmap.height));
        auto bounds = Rectangle2f::fromTopLeft(x, y, width, height);
        return std::make_shared<Sprite>(graphics->createSprite(bounds, region, bitmap.handle));
    }

    TextureService::TextureInfo TextureService::getBitmapInternal(const std::string& bitmapName)
    {
        auto it = bitmapCache.find(bitmapName);
        if (it != bitmapCache.end())
        {
            return it->second;
        }

        auto entry = fileSystem->readFile("bitmaps/" + bitmapName + ".pcx");
        if (!entry)
        {
            throw std::runtime_error("bitmap not found");
        }

        PcxDecoder<std::vector<char>::const_iterator> decoder(entry->begin(), entry->end());

        auto decodedData = decoder.decodeImage();
        auto palette = decoder.decodePalette();

        auto width = decoder.getWidth();
        auto height = decoder.getHeight();

        std::vector<Color> buffer(decodedData.size());
        for (std::size_t i = 0; i < decodedData.size(); ++i)
        {
            auto paletteIndex = static_cast<unsigned char>(decodedData[i]);
            assert(paletteIndex < palette.size());
            buffer[i] = palette[paletteIndex].toColor();
        }

        SharedTextureHandle handle(graphics->createTexture(width, height, buffer));
        TextureInfo info(width, height, handle);
        bitmapCache[bitmapName] = info;
        return info;
    }

    std::shared_ptr<Sprite> TextureService::getMinimap(const std::string& mapName)
    {
        auto it = minimapCache.find(mapName);
        if (it != minimapCache.end())
        {
            return it->second;
        }

        auto tntData = fileSystem->readFile("maps/" + mapName + ".tnt");
        if (!tntData)
        {
            throw std::runtime_error("map tnt not found!");
        }

        boost::interprocess::bufferstream tntStream(tntData->data(), tntData->size());
        TntArchive tnt(&tntStream);
        auto minimap = tnt.readMinimap();

        std::vector<Color> rgbMinimap;
        std::transform(minimap.data.begin(), minimap.data.end(), std::back_inserter(rgbMinimap), [p = palette](unsigned char pixel) {
            assert(pixel >= 0 && pixel <= 255);
            return (*p)[pixel];
        });

        SharedTextureHandle texture(graphics->createTexture(minimap.width, minimap.height, rgbMinimap));
        auto sprite = graphics->createSprite(
            Rectangle2f::fromTopLeft(0.0f, 0.0f, minimap.width, minimap.height),
            Rectangle2f::fromTopLeft(0.0f, 0.0f, 1.0f, 1.0f),
            texture);
        auto spritePtr = std::make_shared<Sprite>(std::move(sprite));

        minimapCache.insert({mapName, spritePtr});

        return spritePtr;
    }

    std::shared_ptr<Sprite> TextureService::getDefaultSprite()
    {
        return defaultSpriteSeries->sprites[0];
    }

    std::shared_ptr<SpriteSeries> TextureService::getFont(const std::string& fontName)
    {
        auto fntBytes = fileSystem->readFile(fontName);
        if (!fntBytes)
        {
            throw std::runtime_error("font not found!");
        }

        boost::interprocess::bufferstream fntStream(fntBytes->data(), fntBytes->size());
        FntArchive fnt(&fntStream);

        auto series = std::make_shared<SpriteSeries>();

        for (unsigned int i = 0; i < 256; ++i)
        {
            std::vector<char> glyphBits(512);
            auto bytesRead = fnt.extract(i, glyphBits.data());

            auto width = bytesRead / 2;

            std::vector<Color> rgbGlyph;
            rgbGlyph.reserve(bytesRead * 8);
            for (unsigned int j = 0; j < bytesRead; ++j)
            {
                auto byte = static_cast<unsigned char>(glyphBits[j]);
                for (unsigned int k = 0; k < 8; ++k)
                {
                    rgbGlyph.push_back(byte & (128u >> k) ? Color(255, 255, 255) : Color(0, 0, 0, 0));
                }
            }

            // read data might have been ragged at the end,
            // the last font in the file is often missing the last byte or two.
            rgbGlyph.resize(width * fnt.glyphHeight());

            SharedTextureHandle texture(graphics->createTexture(width, fnt.glyphHeight(), rgbGlyph));
            auto sprite = std::make_shared<Sprite>(graphics->createSprite(
                Rectangle2f::fromTopLeft(0.0f, 0.0f, width, fnt.glyphHeight()),
                Rectangle2f::fromTopLeft(0.0f, 0.0f, 1.0f, 1.0f),
                texture));

            series->sprites.push_back(sprite);
        }

        return series;
    }

    TextureService::TextureInfo::TextureInfo(unsigned int width, unsigned int height, const SharedTextureHandle& handle)
        : width(width), height(height), handle(handle)
    {
    }
}
