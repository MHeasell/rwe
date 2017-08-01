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

    void decompressRow(std::istream& stream, char* buffer, std::size_t rowLength)
    {
        auto compressedRowLength = readRaw<uint16_t>(stream);

        std::size_t readPos = 0;
        std::size_t writePos = 0;

        while (readPos < compressedRowLength && writePos < rowLength)
        {
            auto mask = readRaw<uint8_t>(stream);
            ++readPos;

            if ((mask & 1) == 1)
            {
                // skip n pixels (transparency)
                auto count = mask >> 1;
                if (writePos + count > rowLength)
                {
                    throw GafException("malformed row");
                }
                writePos += count;
            }
            else if ((mask & 2) == 2)
            {
                // repeat this byte n times
                auto count = (mask >> 2) + 1u;

                if (readPos + 1 > compressedRowLength)
                {
                    throw GafException("malformed row");
                }
                auto val = readRaw<uint8_t>(stream);
                ++readPos;

                if (writePos + count > rowLength)
                {
                    throw GafException("malformed row");
                }
                std::fill_n(buffer + writePos, count, val);
                writePos += count;
            }
            else
            {
                // by default, copy next n bytes
                auto count = (mask >> 2) + 1u;

                if (readPos + count > compressedRowLength)
                {
                    throw GafException("malformed row");
                }
                if (writePos + count > rowLength)
                {
                    throw GafException("malformed row");
                }
                stream.read(buffer + writePos, count);
                readPos += count;
                writePos += count;
            }
        }
    }

    void decompressFrame(std::istream& stream, char* buffer, std::size_t width, std::size_t height)
    {
        for (std::size_t i = 0; i < height; ++i)
        {
            decompressRow(stream, buffer + (i * width), width);
        }
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

    void GafArchive::extract(const GafArchive::Entry& entry, GafReaderAdapter& adapter)
    {
        for (auto offset : entry.frameOffsets)
        {
            _stream->seekg(offset);
            auto frameHeader = readRaw<GafFrameData>(*_stream);
            auto buffer = adapter.beginFrame(frameHeader.width, frameHeader.height);

            _stream->seekg(frameHeader.frameDataOffset);
            if (frameHeader.subframesCount == 0)
            {
                if (frameHeader.compressed == 0)
                {
                    _stream->read(buffer, frameHeader.width * frameHeader.height);
                }
                else
                {
                    decompressFrame(*_stream, buffer, frameHeader.width, frameHeader.height);
                }
            }
            else
            {
                for (std::size_t i = 0; i < frameHeader.subframesCount; ++i)
                {
                    auto subframeOffset = readRaw<uint32_t>(*_stream);
                    auto pos = _stream->tellg();
                    _stream->seekg(subframeOffset);
                    auto subframeHeader = readRaw<GafFrameData>(*_stream);

                    // TODO: support subframes
                    throw std::logic_error("subframes not implemented");

                    _stream->seekg(pos);
                }
            }

            adapter.endFrame();
        }
    }

    GafException::GafException(const char* message) : runtime_error(message) {}
}
