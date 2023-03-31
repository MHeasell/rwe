#include "gaf_util.h"

#include <memory>
#include <rwe/io/io_util.h>

namespace rwe
{
    GafException::GafException(const char* message) : runtime_error(message) {}

    void decompressRow(std::istream& stream, char* buffer, std::size_t rowLength, char transparencyIndex)
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
                std::fill_n(buffer + writePos, count, transparencyIndex);
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

        std::fill_n(buffer, rowLength - writePos, transparencyIndex);
    }

    void decompressFrame(std::istream& stream, char* buffer, std::size_t width, std::size_t height, char transparencyIndex)
    {
        for (std::size_t i = 0; i < height; ++i)
        {
            decompressRow(stream, buffer + (i * width), width, transparencyIndex);
        }
    }

    void extractGafEntry(std::istream* _stream, const std::vector<GafFrameEntry>& frameEntries, GafReaderAdapter& adapter)
    {
        for (auto entry : frameEntries)
        {
            _stream->seekg(entry.frameDataOffset);
            auto frameHeader = readRaw<GafFrameData>(*_stream);
            adapter.beginFrame(entry, frameHeader);

            _stream->seekg(frameHeader.frameDataOffset);
            if (frameHeader.subframesCount == 0)
            {
                auto buffer = std::make_unique<char[]>(frameHeader.width * frameHeader.height);
                if (frameHeader.compressed == 0)
                {
                    _stream->read(buffer.get(), frameHeader.width * frameHeader.height);
                }
                else
                {
                    decompressFrame(*_stream, buffer.get(), frameHeader.width, frameHeader.height, frameHeader.transparencyIndex);
                }

                GafReaderAdapter::LayerData layer{
                    frameHeader.posX,
                    frameHeader.posY,
                    frameHeader.width,
                    frameHeader.height,
                    frameHeader.transparencyIndex,
                    buffer.get(),
                };

                adapter.frameLayer(layer);
            }
            else
            {
                for (std::size_t i = 0; i < frameHeader.subframesCount; ++i)
                {
                    auto subframeOffset = readRaw<uint32_t>(*_stream);
                    auto pos = _stream->tellg();
                    _stream->seekg(subframeOffset);
                    auto subframeHeader = readRaw<GafFrameData>(*_stream);
                    auto buffer = std::make_unique<char[]>(subframeHeader.width * subframeHeader.height);

                    _stream->seekg(subframeHeader.frameDataOffset);
                    if (subframeHeader.compressed == 0)
                    {
                        _stream->read(buffer.get(), subframeHeader.width * subframeHeader.height);
                    }
                    else
                    {
                        decompressFrame(*_stream, buffer.get(), subframeHeader.width, subframeHeader.height, subframeHeader.transparencyIndex);
                    }

                    GafReaderAdapter::LayerData layer{
                        subframeHeader.posX,
                        subframeHeader.posY,
                        subframeHeader.width,
                        subframeHeader.height,
                        subframeHeader.transparencyIndex,
                        buffer.get(),
                    };

                    adapter.frameLayer(layer);

                    _stream->seekg(pos);
                }
            }

            adapter.endFrame();
        }
    }
}
