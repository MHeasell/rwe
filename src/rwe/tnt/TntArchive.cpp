#include "TntArchive.h"

#include <array>

namespace rwe
{
    template <typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
    }

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
}
