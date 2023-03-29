#include "atlas_util.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/BoxTreeSplit.h>
#include <rwe/io/gaf/GafArchive.h>
#include <rwe/util/Index.h>
#include <rwe/util/match.h>

namespace rwe
{
    struct FrameInfo
    {
        std::string name;
        unsigned int frameNumber;
        Grid<Color> data;

        FrameInfo(const std::string& name, unsigned int frameNumber, unsigned int width, unsigned int height)
            : name(name), frameNumber(frameNumber), data(width, height)
        {
        }
    };

    class FrameListGafAdapter : public GafReaderAdapter
    {
    private:
        const ColorPalette* palette;
        std::vector<FrameInfo>* frames;
        const std::string* entryName;
        FrameInfo* frameInfo;
        GafFrameData currentFrameHeader;
        unsigned int frameNumber{0};

    public:
        explicit FrameListGafAdapter(const ColorPalette* palette, std::vector<FrameInfo>* frames, const std::string* entryName)
            : palette(palette), frames(frames), entryName(entryName)
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

                    frameInfo->data.set(outPosX, outPosY, (*palette)[colorIndex]);
                }
            }
        }

        void endFrame() override
        {
            ++frameNumber;
        }
    };

    struct AtlasItemFrame
    {
        FrameInfo* frameInfo;
    };
    struct AtlasItemColor
    {
        unsigned int colorIndex;
    };
    using AtlasItem = std::variant<AtlasItemFrame, AtlasItemColor>;


    std::pair<std::unordered_map<std::string, Rectangle2f>, std::vector<SharedTextureHandle>> createTeamColorAtlases(
        AbstractVirtualFileSystem& vfs,
        GraphicsContext& graphics,
        const ColorPalette& palette)
    {
        auto bytes = vfs.readFile("textures/LOGOS.GAF");
        if (!bytes)
        {
            throw std::runtime_error("textures/LOGOS.GAF could not be read");
        }

        boost::interprocess::bufferstream stream(bytes->data(), bytes->size());
        GafArchive gaf(&stream);

        std::vector<std::pair<std::string, std::vector<FrameInfo>>> entries;
        for (const auto& e : gaf.entries())
        {
            std::vector<FrameInfo> frames;
            FrameListGafAdapter adapter(&palette, &frames, &e.name);
            gaf.extract(e, adapter);
            entries.emplace_back(e.name, std::move(frames));
        }

        // FIXME: should probably assert that
        // 1. each logo entry has 10 frames
        // 2. all logo frames are the same size

        std::vector<Index> entryRefs;
        entryRefs.reserve(entries.size());
        for (Index i = 0; i < getSize(entries); ++i)
        {
            entryRefs.push_back(i);
        }

        auto packInfo = packGridsGeneric<Index>(entryRefs, [&entries](Index i) {
            const auto& firstFrame = entries.at(i).second.at(0);
            return Size(roundUpToPowerOfTwo(firstFrame.data.getWidth()), roundUpToPowerOfTwo(firstFrame.data.getHeight()));
        });

        std::unordered_map<std::string, Rectangle2f> atlasMap;
        for (const auto& e : packInfo.entries)
        {
            const auto& entryName = entries.at(e.value).first;
            const auto& firstFrame = entries.at(e.value).second.at(0);

            auto left = static_cast<float>(e.x) / static_cast<float>(packInfo.width);
            auto top = static_cast<float>(e.y) / static_cast<float>(packInfo.height);
            auto right = static_cast<float>(e.x + firstFrame.data.getWidth()) / static_cast<float>(packInfo.width);
            auto bottom = static_cast<float>(e.y + firstFrame.data.getHeight()) / static_cast<float>(packInfo.height);
            auto bounds = Rectangle2f::fromTLBR(top, left, bottom, right);

            atlasMap.insert({entryName, bounds});
        }

        std::vector<SharedTextureHandle> atlases;
        for (int i = 0; i < 10; ++i)
        {
            Grid<Color> atlas(packInfo.width, packInfo.height);
            for (const auto& e : packInfo.entries)
            {
                const auto& frames = entries.at(e.value).second;
                if (getSize(frames) <= i)
                {
                    // skip if there is no frame for this team
                    continue;
                }
                // Sometimes not all frames in a single logos.gaf entry are the same size.
                // For example, in totala1.hpi/textures/LOGOS.GAF/Arm32Lt, frame 2 has size 32x33,
                // but the other frames are all 32x32.
                // When we created the atlas we used the size of frame 0,
                // so we'll restrict the maximum size to that here.
                // Otherwise we may write over other textures in the atlas or go out of bounds.
                const auto& firstFrame = entries.at(e.value).second.at(0);
                const auto& frame = entries.at(e.value).second.at(i);
                atlas.replace(
                    e.x,
                    e.y,
                    std::min(firstFrame.data.getWidth(), frame.data.getWidth()),
                    std::min(firstFrame.data.getHeight(), frame.data.getHeight()),
                    frame.data);
            }
            atlases.emplace_back(graphics.createTexture(atlas));
        }

        return std::make_pair(std::move(atlasMap), std::move(atlases));
    }

    TextureAtlasInfo createTextureAtlases(AbstractVirtualFileSystem* vfs, GraphicsContext* graphics, const ColorPalette* palette)
    {
        auto gafs = vfs->getFileNames("textures", ".gaf");

        std::vector<FrameInfo> frames;

        // load all the textures into memory
        for (const auto& gafName : gafs)
        {
            // skip team-color textures -- we'll handle these separately
            if (toUpper(gafName) == "LOGOS.GAF")
            {
                continue;
            }

            auto bytes = vfs->readFile("textures/" + gafName);
            if (!bytes)
            {
                throw std::runtime_error("File in listing could not be read: " + gafName);
            }

            boost::interprocess::bufferstream stream(bytes->data(), bytes->size());
            GafArchive gaf(&stream);


            for (const auto& e : gaf.entries())
            {
                FrameListGafAdapter adapter(palette, &frames, &e.name);
                gaf.extract(e, adapter);
            }
        }

        // figure out how to pack the textures into an atlas
        std::vector<AtlasItem> frameRefs;
        frameRefs.reserve(frames.size());
        for (auto& f : frames)
        {
            // just drop multi-frame (animated) textures for now
            // and use the first frame.
            // TODO: support animated textures
            if (f.frameNumber != 0)
            {
                continue;
            }

            frameRefs.emplace_back(AtlasItemFrame{&f});
        }

        for (unsigned int i = 0; i < palette->size(); ++i)
        {
            frameRefs.emplace_back(AtlasItemColor{i});
        }

        // For packing, round the area occupied by the texture up to the nearest power of two.
        // This is required to prevent texture bleeding when shrinking the atlas for mipmaps.
        auto packInfo = packGridsGeneric<AtlasItem>(frameRefs, [](const AtlasItem& item) {
            return match(
                item,
                [](const AtlasItemFrame& f) {
                    return Size(roundUpToPowerOfTwo(f.frameInfo->data.getWidth()), roundUpToPowerOfTwo(f.frameInfo->data.getHeight()));
                },
                [](const AtlasItemColor&) {
                    return Size(1, 1);
                });
        });

        // pack the textures
        Grid<Color> atlas(packInfo.width, packInfo.height);
        std::unordered_map<std::string, Rectangle2f> atlasMap;
        std::vector<Vector2f> atlasColorMap(palette->size());

        for (const auto& e : packInfo.entries)
        {
            match(
                e.value,
                [&](const AtlasItemFrame& f) {
                    auto left = static_cast<float>(e.x) / static_cast<float>(packInfo.width);
                    auto top = static_cast<float>(e.y) / static_cast<float>(packInfo.height);
                    auto right = static_cast<float>(e.x + f.frameInfo->data.getWidth()) / static_cast<float>(packInfo.width);
                    auto bottom = static_cast<float>(e.y + f.frameInfo->data.getHeight()) / static_cast<float>(packInfo.height);
                    auto bounds = Rectangle2f::fromTLBR(top, left, bottom, right);

                    atlasMap.insert({f.frameInfo->name, bounds});

                    atlas.replace(e.x, e.y, f.frameInfo->data);
                },
                [&](const AtlasItemColor& c) {
                    atlasColorMap[c.colorIndex] = Vector2f((e.x + 0.5f) / static_cast<float>(packInfo.width), (e.y + 0.5f) / static_cast<float>(packInfo.height));
                    atlas.set(e.x, e.y, (*palette)[c.colorIndex]);
                });
        }

        SharedTextureHandle atlasTexture(graphics->createTexture(atlas));

        auto teamColorInfo = createTeamColorAtlases(*vfs, *graphics, *palette);

        return TextureAtlasInfo{
            std::move(atlasTexture),
            std::move(atlasMap),
            std::move(atlasColorMap),
            std::move(teamColorInfo.second),
            std::move(teamColorInfo.first)};
    }

}
