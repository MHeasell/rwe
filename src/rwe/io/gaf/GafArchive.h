#pragma once

#include <functional>
#include <istream>
#include <optional>
#include <rwe/io/gaf/GafReaderAdapter.h>
#include <rwe/io/gaf/gaf_headers.h>
#include <vector>

namespace rwe
{
    class GafArchive
    {
    public:
        struct Entry
        {
            std::string name;
            uint16_t unknown1;
            uint32_t unknown2;
            std::vector<GafFrameEntry> frameOffsets;
        };

    private:
        std::vector<Entry> _entries;
        std::istream* _stream;

    public:
        explicit GafArchive(std::istream* stream);

        const std::vector<Entry>& entries() const;

        std::optional<std::reference_wrapper<const Entry>> findEntry(const std::string& name) const;

        void extract(const Entry& entry, GafReaderAdapter& adapter);
    };
}
