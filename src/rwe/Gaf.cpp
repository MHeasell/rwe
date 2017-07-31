#include "Gaf.h"

#include <algorithm>

namespace rwe
{
    template <typename T>
    T readRaw(std::istream& stream)
    {
        T val;
        stream.read(reinterpret_cast<char*>(&val), sizeof(T));
        return val;
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
    }

    GafArchive::Entry GafArchive::readEntry(std::istream& stream) const
    {
        auto entry = readRaw<GafEntry>(stream);

        auto nullPos = std::find(entry.name, entry.name + GafMaxNameLength, '\0');
        auto nameLength = nullPos - entry.name;
        std::string name(reinterpret_cast<char*>(entry.name), nameLength);

        std::vector<std::size_t> frames;
        frames.reserve(entry.frames);

        for (std::size_t i = 0; i < entry.frames; ++i)
        {
            auto frameEntry = readRaw<GafFrameEntry>(stream);
            frames.emplace_back(frameEntry.frameDataOffset);
        }

        return Entry { std::move(name), std::move(frames) };
    }

    boost::optional<const GafArchive::Entry&> GafArchive::findEntry(const std::string& name) const
    {
        auto pos = std::find_if(_entries.begin(), _entries.end(), [&name](const Entry& e) { return e.name == name; });

        if (pos == _entries.end())
        {
            return boost::none;
        }

        return *pos;
    }

    GafException::GafException(const char* message) : runtime_error(message) {}
}
