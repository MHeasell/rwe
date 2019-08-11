#pragma once

#include <cstdint>
#include <functional>
#include <istream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace rwe
{
    static const unsigned int GafVersionNumber = 0x00010100;

    static const unsigned int GafMaxNameLength = 32;

    class GafException : public std::runtime_error
    {
    public:
        explicit GafException(const char* message);
    };

#pragma pack(1) // don't pad members

    struct GafHeader
    {
        /** Version number. */
        uint32_t version;

        /** The number of entries in the file. */
        uint32_t entries;

        uint32_t unknown1;
    };

    struct GafEntry
    {
        /** Number of frames in this entry. */
        uint16_t frames;

        uint16_t unknown1;

        uint32_t unknown2;

        /** Name of the entry. */
        uint8_t name[GafMaxNameLength];
    };

    struct GafFrameEntry
    {
        uint32_t frameDataOffset;
        uint32_t unknown1;
    };

    struct GafFrameData
    {
        /** Width of the frame in pixels. */
        uint16_t width;

        /** Height of the frame in pixels. */
        uint16_t height;

        int16_t posX;
        int16_t posY;

        /** The color palette index that should be treated as transparent. */
        uint8_t transparencyIndex;

        /** 1 if compressed, 0 otherwise. */
        uint8_t compressed;

        /**
         * If this is zero, the frame data is raw pixel data.
         * Otherwise, the frame data is an array of this many subframes.
         */
        uint16_t subframesCount;

        uint32_t unknown2;

        /** Offset to the start of the frame data in the file. */
        uint32_t frameDataOffset;

        uint32_t unknown3;
    };

#pragma pack()

    class GafReaderAdapter
    {
    public:
        struct LayerData
        {
            int x;
            int y;
            unsigned int width;
            unsigned int height;
            unsigned char transparencyKey;
            char* data;
        };

    public:
        virtual void beginFrame(const GafFrameData& header) = 0;
        virtual void frameLayer(const LayerData& data) = 0;
        virtual void endFrame() = 0;
    };

    class GafArchive
    {
    public:
        struct Entry
        {
            std::string name;
            std::vector<std::size_t> frameOffsets;
        };

    private:
        std::vector<Entry> _entries;
        std::istream* _stream;

    public:
        explicit GafArchive(std::istream* stream);

        const std::vector<Entry>& entries() const;

        std::optional<std::reference_wrapper<const Entry>> findEntry(const std::string& name) const;

        void extract(const Entry& entry, GafReaderAdapter& adapter);

    private:
        Entry readEntry(std::istream& stream) const;
    };
}
