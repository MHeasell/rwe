#pragma once

#include <istream>
#include <rwe/io/gaf/GafReaderAdapter.h>
#include <rwe/io/gaf/gaf_headers.h>
#include <vector>

namespace rwe
{
    class GafException : public std::runtime_error
    {
    public:
        explicit GafException(const char* message);
    };

    void decompressRow(std::istream& stream, char* buffer, std::size_t rowLength, char transparencyIndex);

    void decompressFrame(std::istream& stream, char* buffer, std::size_t width, std::size_t height, char transparencyIndex);

    void extractGafEntry(std::istream* _stream, const std::vector<GafFrameEntry>& frameEntries, GafReaderAdapter& adapter);
}
