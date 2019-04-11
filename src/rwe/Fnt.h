#ifndef RWE_FNT_H
#define RWE_FNT_H

#include <array>
#include <cstdint>
#include <istream>

namespace rwe
{
    struct FntHeader
    {
        uint16_t glyphHeight;
        uint16_t unknown2;
    };

    class FntArchive
    {
    private:
        FntHeader _header;
        std::array<uint16_t, 256> _entries;
        std::istream* _stream;

    public:
        explicit FntArchive(std::istream* stream);

        unsigned int glyphHeight() const;

        /**
         * Writes glyph data out to the given buffer.
         * Returns the number of bytes written.
         * The data is 1-bit per pixel bitmap with 16 rows.
         * Row width can be found by dividing the return value by 16.
         *
         * Guaranteed to write no more than 510 bytes.
         *
         * If there is no glyph for the given index (from 0 to 255),
         * returns 0 and writes no data to the buffer.
         */
        unsigned int extract(unsigned int index, char* buffer);
    };
}

#endif
