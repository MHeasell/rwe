#include "TntArchive.h"

#include <array>
#include <cassert>
#include <rwe/io_utils.h>

namespace rwe
{
    TntException::TntException(const std::string& __arg) : runtime_error(__arg)
    {
    }

    TntException::TntException(const char* string) : runtime_error(string)
    {
    }

    TntArchive::TntArchive(std::istream* _stream) : stream(_stream)
    {
        header = readRaw<TntHeader>(*stream);
        if (header.magicNumber != TntMagicNumber)
        {
            throw TntException("Invalid TNT version number");
        }
    }

    void TntArchive::readTiles(std::function<void(const char*)> tileCallback)
    {
        stream->seekg(header.tileGraphicsOffset);

        std::array<char, 32 * 32> buffer{};

        for (unsigned int i = 0; i < header.numberOfTiles; ++i)
        {
            stream->read(buffer.data(), 32 * 32);
            tileCallback(buffer.data());
        }
    }

    void TntArchive::readFeatures(std::function<void(const std::string&)> featureCallback)
    {
        stream->seekg(header.featuresOffset);

        std::string str;
        str.reserve(128);

        for (unsigned int i = 0; i < header.numberOfFeatures; ++i)
        {
            auto feature = readRaw<TntFeature>(*stream);
            auto nullIt = std::find(feature.name, feature.name + 128, '\0');
            str.erase();
            str.append(feature.name, nullIt);
            featureCallback(str);
        }
    }

    const TntHeader& TntArchive::getHeader() const
    {
        return header;
    }

    void TntArchive::readMapData(uint16_t* outputBuffer)
    {
        stream->seekg(header.mapDataOffset);
        stream->read(reinterpret_cast<char*>(outputBuffer), (header.width / 2) * (header.height / 2) * sizeof(uint16_t));
    }

    void TntArchive::readMapAttributes(TntTileAttributes* outputBuffer)
    {
        stream->seekg(header.mapAttributesOffset);
        stream->read(reinterpret_cast<char*>(outputBuffer), header.width * header.height * sizeof(TntTileAttributes));
    }

    struct MinimapSize
    {
        unsigned int width;
        unsigned int height;
    };

    MinimapSize getMinimapActualSize(const std::vector<char>& data, unsigned int width, unsigned int height)
    {
        unsigned int realWidth = width;
        unsigned int realHeight = height;

        while (realWidth > 0 && data[realWidth - 1] == TntMinimapVoidByte)
        {
            --realWidth;
        }

        while (realHeight > 0 && data[((realHeight - 1) * width)] == TntMinimapVoidByte)
        {
            --realHeight;
        }

        return MinimapSize{realWidth, realHeight};
    }

    std::vector<char> trimMinimapBytes(const std::vector<char>& data, unsigned int width, unsigned int height, unsigned int newWidth, unsigned int newHeight)
    {
        assert(newWidth <= width);
        assert(newHeight <= height);

        std::vector<char> newData(newWidth * newHeight);
        for (unsigned int y = 0; y < newHeight; ++y)
        {
            for (unsigned int x = 0; x < newWidth; ++x)
            {
                newData[(y * newWidth) + x] = data[(y * width) + x];
            }
        }

        return newData;
    }

    TntMinimapInfo TntArchive::readMinimap()
    {
        stream->seekg(header.minimapOffset);
        auto minimapHeader = readRaw<TntMinimapHeader>(*stream);

        std::vector<char> buffer(minimapHeader.width * minimapHeader.height);
        stream->read(buffer.data(), minimapHeader.width * minimapHeader.height);

        auto realSize = getMinimapActualSize(buffer, minimapHeader.width, minimapHeader.height);

        auto resizedBuffer = trimMinimapBytes(buffer, minimapHeader.width, minimapHeader.height, realSize.width, realSize.height);

        return TntMinimapInfo{realSize.width, realSize.height, std::move(resizedBuffer)};
    }
}
