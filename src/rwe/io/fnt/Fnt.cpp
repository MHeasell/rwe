#include "Fnt.h"
#include <rwe/io/io_util.h>

namespace rwe
{
    FntArchive::FntArchive(std::istream* stream) : _entries()
    {
        _header = readRaw<FntHeader>(*stream);

        for (auto& e : _entries)
        {
            e = readRaw<uint16_t>(*stream);
        }

        _stream = stream;
    }

    unsigned int FntArchive::extract(unsigned int index, char* buffer)
    {
        auto ptr = _entries.at(index);
        if (ptr == 0)
        {
            return 0;
        }

        _stream->seekg(ptr);
        auto length = readRaw<uint8_t>(*_stream);
        _stream->read(buffer, length * 2);
        return _stream->gcount();
    }

    unsigned int FntArchive::glyphHeight() const
    {
        return _header.glyphHeight;
    }
}
