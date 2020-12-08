#include "GafArchive.h"
#include <algorithm>
#include <memory>
#include <rwe/io/gaf/gaf_util.h>
#include <rwe/io_utils.h>
#include <rwe/rwe_string.h>

namespace rwe
{
    GafArchive::Entry readEntry(std::istream& stream)
    {
        auto entry = readRaw<GafEntry>(stream);

        auto nullPos = std::find(entry.name, entry.name + GafMaxNameLength, '\0');
        auto nameLength = nullPos - entry.name;
        std::string name(reinterpret_cast<char*>(entry.name), nameLength);

        std::vector<GafFrameEntry> frames;
        frames.reserve(entry.frames);

        for (std::size_t i = 0; i < entry.frames; ++i)
        {
            auto frameEntry = readRaw<GafFrameEntry>(stream);
            frames.emplace_back(frameEntry);
        }

        return GafArchive::Entry{std::move(name), entry.unknown1, entry.unknown2, std::move(frames)};
    }

    const std::vector<GafArchive::Entry>& GafArchive::entries() const
    {
        return _entries;
    }

    GafArchive::GafArchive(std::istream* stream)
    {
        auto header = readRaw<GafHeader>(*stream);
        if (header.version != GafVersionNumber)
        {
            throw GafException("Invalid GAF version number");
        }

        _entries.reserve(header.entries);

        for (std::size_t i = 0; i < header.entries; ++i)
        {
            auto pointer = readRaw<uint32_t>(*stream);
            auto previous = stream->tellg();

            stream->seekg(pointer);
            _entries.push_back(readEntry(*stream));
            stream->seekg(previous);
        }

        _stream = stream;
    }

    std::optional<std::reference_wrapper<const GafArchive::Entry>> GafArchive::findEntry(const std::string& name) const
    {
        auto pos = std::find_if(_entries.begin(), _entries.end(), [&name](const Entry& e) { return toUpper(e.name) == toUpper(name); });

        if (pos == _entries.end())
        {
            return std::nullopt;
        }

        return *pos;
    }

    void GafArchive::extract(const GafArchive::Entry& entry, GafReaderAdapter& adapter)
    {
        extractGafEntry(_stream, entry.frameOffsets, adapter);
    }
}
