#include <boost/functional/hash.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <iostream>
#include <memory>
#include <png++/png.hpp>
#include <rwe/BoxTreeSplit.h>
#include <rwe/ColorPalette.h>
#include <rwe/geometry/Rectangle2f.h>
#include <rwe/io/gaf/Gaf.h>
#include <rwe/math/rwe_math.h>
#include <rwe/util.h>
#include <rwe/vfs/AbstractVirtualFileSystem.h>
#include <rwe/vfs/CompositeVirtualFileSystem.h>
#include <string>
#include <vector>

namespace rwe
{
    using FrameId = std::pair<std::string, unsigned int>;
}

namespace std
{
    template <>
    struct hash<rwe::FrameId>
    {
        std::size_t operator()(const rwe::FrameId& f) const noexcept
        {
            return boost::hash<rwe::FrameId>()(f);
        }
    };
}

namespace rwe
{
    struct FrameInfo
    {
        std::string name;
        unsigned int frameNumber;
        Grid<char> data;

        FrameInfo(const std::string& name, unsigned int frameNumber, unsigned int width, unsigned int height)
            : name(name), frameNumber(frameNumber), data(width, height)
        {
        }
    };

    class FrameListGafAdapter : public GafReaderAdapter
    {
    private:
        std::vector<FrameInfo>* frames;
        const std::string* entryName;
        FrameInfo* frameInfo;
        GafFrameData currentFrameHeader;
        unsigned int frameNumber{0};

    public:
        explicit FrameListGafAdapter(std::vector<FrameInfo>* frames, const std::string* entryName)
            : frames(frames),
              entryName(entryName)
        {
        }

        void beginFrame(const GafFrameEntry& entry, const GafFrameData& header) override
        {
            frameInfo = &(frames->emplace_back(*entryName, frameNumber, header.width, header.height));
            currentFrameHeader = header;
        }

        void frameLayer(const LayerData& data) override
        {
            for (std::size_t y = 0; y < data.height; ++y)
            {
                for (std::size_t x = 0; x < data.width; ++x)
                {
                    auto outPosX = static_cast<int>(x) - (data.x - currentFrameHeader.posX);
                    auto outPosY = static_cast<int>(y) - (data.y - currentFrameHeader.posY);

                    if (outPosX < 0 || outPosX >= currentFrameHeader.width || outPosY < 0 || outPosY >= currentFrameHeader.height)
                    {
                        throw std::runtime_error("frame coordinate out of bounds");
                    }

                    auto colorIndex = static_cast<unsigned char>(data.data[(y * data.width) + x]);
                    if (colorIndex == data.transparencyKey)
                    {
                        continue;
                    }

                    frameInfo->data.set(outPosX, outPosY, colorIndex);
                }
            }
        }

        void endFrame() override
        {
            ++frameNumber;
        }
    };

    Grid<Color> doThing(AbstractVirtualFileSystem* vfs, const ColorPalette* palette)
    {
        auto gafs = vfs->getFileNames("textures", ".gaf");

        std::vector<FrameInfo> frames;

        // load all the textures into memory
        for (const auto& gafName : gafs)
        {
            auto bytes = vfs->readFile("textures/" + gafName);
            if (!bytes)
            {
                throw std::runtime_error("File in listing could not be read: " + gafName);
            }

            boost::interprocess::bufferstream stream(bytes->data(), bytes->size());
            GafArchive gaf(&stream);

            for (const auto& e : gaf.entries())
            {
                FrameListGafAdapter adapter(&frames, &e.name);
                gaf.extract(e, adapter);
            }
        }

        // figure out how to pack the textures into an atlas
        std::vector<FrameInfo*> frameRefs;
        frameRefs.reserve(frames.size());
        for (auto& f : frames)
        {
            frameRefs.push_back(&f);
        }

        // For packing, round the area occupied by the texture up to the nearest power of two.
        // This is required to prevent texture bleeding when shrinking the atlas for mipmaps.
        auto packInfo = packGridsGeneric<FrameInfo*>(frameRefs, [](const FrameInfo* f) {
            return Size(roundUpToPowerOfTwo(f->data.getWidth()), roundUpToPowerOfTwo(f->data.getHeight()));
        });

        // pack the textures
        Grid<Color> atlas(packInfo.width, packInfo.height);
        std::unordered_map<FrameId, Rectangle2f> atlasMap;

        for (const auto& e : packInfo.entries)
        {
            FrameId id(e.value->name, e.value->frameNumber);

            auto left = static_cast<float>(e.x) / static_cast<float>(packInfo.width);
            auto top = static_cast<float>(e.y) / static_cast<float>(packInfo.height);
            auto right = static_cast<float>(e.x + e.value->data.getWidth()) / static_cast<float>(packInfo.width);
            auto bottom = static_cast<float>(e.y + e.value->data.getHeight()) / static_cast<float>(packInfo.height);
            auto bounds = Rectangle2f::fromTLBR(top, left, bottom, right);

            atlasMap.insert({id, bounds});

            atlas.transformAndReplace<char>(e.x, e.y, e.value->data, [palette](char v) {
                return (*palette)[static_cast<unsigned char>(v)];
            });
        }

        return atlas;
    }

    void dumpImage(const Grid<Color>& g, const std::string& outFile)
    {
        auto width = static_cast<unsigned int>(g.getWidth());
        auto height = static_cast<unsigned int>(g.getHeight());
        png::image<png::rgb_pixel> image(width, height);
        for (png::uint_32 y = 0; y < height; ++y)
        {
            for (png::uint_32 x = 0; x < width; ++x)
            {
                Color px = g.get(x, y);
                image[y][x] = png::rgb_pixel(px.r, px.g, px.b);
            }
        }

        image.write(outFile);
    }
}

int main(int argc, char* argv[])
{
    auto searchPath = rwe::getSearchPath();
    if (!searchPath)
    {
        std::cerr << "Failed to determine data search path" << std::endl;
        return 1;
    }

    auto vfs = rwe::constructVfs(*searchPath);

    auto paletteBytes = vfs.readFile("palettes/PALETTE.PAL");
    if (!paletteBytes)
    {
        std::cerr << "Couldn't find palette" << std::endl;
        return 1;
    }

    auto palette = rwe::readPalette(*paletteBytes);
    if (!palette)
    {
        std::cerr << "Couldn't read palette" << std::endl;
        return 1;
    }

    auto packedTextures = rwe::doThing(&vfs, &*palette);

    dumpImage(packedTextures, "packed_textures.png");

    return 0;
}
