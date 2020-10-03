#pragma once

#include <cstdint>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace rwe
{
    static const unsigned int TntMagicNumber = 0x2000;

    static const uint8_t TntMinimapVoidByte = 0x64;

#pragma pack(1)
    struct TntHeader
    {
        uint32_t magicNumber;

        /**
         * The width of the map in 16-pixel units as used for feature/height information.
         * To get the width in 32-pixel units (the size of a tile), divide by 2.
         */
        uint32_t width;

        /**
         * The height of the map in 16-pixel units as used for feature/height information.
         * To get the height in 32-pixel units (the size of a tile), divide by 2.
         */
        uint32_t height;

        uint32_t mapDataOffset;
        uint32_t mapAttributesOffset;
        uint32_t tileGraphicsOffset;
        uint32_t numberOfTiles;
        uint32_t numberOfFeatures;
        uint32_t featuresOffset;
        uint32_t seaLevel;
        uint32_t minimapOffset;
        uint32_t unknown1;
        uint32_t pad1;
        uint32_t pad2;
        uint32_t pad3;
        uint32_t pad4;
    };

    struct TntTileAttributes
    {
        static const uint16_t FeatureNone = 0xFFFF;
        static const uint16_t FeatureVoid = 0xFFFC;
        static const uint16_t FeatureUnknown = 0xFFFE;

        uint8_t height;
        uint16_t feature;
        uint8_t pad1;
    };

    struct TntFeature
    {
        uint32_t index;
        uint8_t name[128];
    };

    struct TntMinimapHeader
    {
        uint32_t width;
        uint32_t height;
    };
#pragma pack()

    struct TntMinimapInfo
    {
        unsigned int width;
        unsigned int height;
        std::vector<char> data;
    };

    class TntException : public std::runtime_error
    {
    public:
        explicit TntException(const std::string& __arg);

        explicit TntException(const char* string);
    };

    class TntArchive
    {
    private:
        TntHeader header;

        std::istream* stream;

    public:
        explicit TntArchive(std::istream* _stream);

        TntMinimapInfo readMinimap();

        void readTiles(std::function<void(const char*)> tileCallback);

        void readFeatures(std::function<void(const std::string&)> featureCallback);

        void readMapData(uint16_t* outputBuffer);

        void readMapAttributes(TntTileAttributes* outputBuffer);

        const TntHeader& getHeader() const;
    };
}
