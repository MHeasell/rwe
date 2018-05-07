#include "MeshService.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/BoxTreeSplit.h>
#include <rwe/Gaf.h>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/math/rwe_math.h>
#include <rwe/rwe_string.h>

namespace rwe
{
    Vector3f vertexToVector(const _3do::Vertex& v)
    {
        return Vector3f(
            fromFixedPoint(v.x),
            fromFixedPoint(v.y),
            fromFixedPoint(-v.z)); // flip to convert from left-handed to right-handed
    }

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

        void beginFrame(const GafFrameData& header) override
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

    MeshService MeshService::createMeshService(AbstractVirtualFileSystem* vfs, GraphicsContext* graphics, const ColorPalette* palette)
    {
        auto gafs = vfs->getFileNames("textures", ".gaf");

        std::vector<FrameInfo> frames;
        std::unordered_map<std::string, TextureAttributes> attribs;

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

            bool isTeamDependent = toUpper(gafName) == "LOGOS.GAF";

            for (const auto& e : gaf.entries())
            {
                attribs[e.name] = TextureAttributes{isTeamDependent};
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

            atlas.transformAndReplaceArea<char>(e.x, e.y, e.value->data, [palette](char v) {
                return (*palette)[static_cast<unsigned char>(v)];
            });
        }

        SharedTextureHandle atlasTexture(graphics->createTexture(atlas));

        return MeshService(vfs, palette, std::move(atlasTexture), std::move(atlasMap), std::move(attribs));
    }

    MeshService::MeshService(
        AbstractVirtualFileSystem* vfs,
        const ColorPalette* palette,
        SharedTextureHandle&& atlas,
        std::unordered_map<FrameId, Rectangle2f>&& atlasMap,
        std::unordered_map<std::string, TextureAttributes> textureAttributesMap)
        : vfs(vfs),
          palette(palette),
          atlas(std::move(atlas)),
          atlasMap(std::move(atlasMap)),
          textureAttributesMap(std::move(textureAttributesMap))
    {
    }

    Mesh MeshService::meshFrom3do(const _3do::Object& o, unsigned int teamColor)
    {
        Mesh m;
        m.texture = getMeshTextureAtlas();

        for (const auto& p : o.primitives)
        {
            // handle textured quads
            if (p.vertices.size() == 4 && p.textureName)
            {
                auto textureBounds = getTextureRegion(*(p.textureName), teamColor);

                Mesh::Triangle t0(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[1]]), textureBounds.topRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.topLeft()));

                Mesh::Triangle t1(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[3]]), textureBounds.bottomLeft()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.topLeft()));

                m.faces.push_back(t0);
                m.faces.push_back(t1);

                continue;
            }

            // handle other polygon types
            if (p.vertices.size() >= 3 && p.colorIndex)
            {
                const auto& first = vertexToVector(o.vertices[p.vertices.front()]);
                for (unsigned int i = p.vertices.size() - 1; i >= 2; --i)
                {
                    const auto& second = vertexToVector(o.vertices[p.vertices[i]]);
                    const auto& third = vertexToVector(o.vertices[p.vertices[i - 1]]);
                    Mesh::Triangle t(
                        Mesh::Vertex(first, Vector2f(0.0f, 0.0f)),
                        Mesh::Vertex(second, Vector2f(0.0f, 0.0f)),
                        Mesh::Vertex(third, Vector2f(0.0f, 0.0f)),
                        (*palette)[*p.colorIndex]);
                    m.colorFaces.push_back(t);
                }

                continue;
            }

            // just ignore degenerate faces (0, 1 or 2 vertices)
        }

        return m;
    }

    template <typename TriRange>
    float getHeight(const TriRange& r)
    {
        auto height = -std::numeric_limits<float>::infinity();
        for (const auto& t : r)
        {
            for (const auto& f : std::array<Mesh::Vertex, 3>{t.a, t.b, t.c})
            {
                if (f.position.y > height)
                {
                    height = f.position.y;
                }
            }
        }

        return height;
    }

    float getMeshHeight(const Mesh& mesh)
    {
        return std::max(getHeight(mesh.faces), getHeight(mesh.colorFaces));
    }

    MeshService::InnerUnitMeshInfo MeshService::unitMeshFrom3do(const _3do::Object& o, unsigned int teamColor)
    {
        UnitMesh m;
        m.origin = vertexToVector(_3do::Vertex(o.x, o.y, o.z));
        m.name = o.name;
        auto mesh = meshFrom3do(o, teamColor);
        auto height = m.origin.y + getMeshHeight(mesh);
        m.mesh = std::make_shared<ShaderMesh>(convertMesh(mesh));

        for (const auto& c : o.children)
        {
            auto info = unitMeshFrom3do(c, teamColor);
            auto childHeight = m.origin.y + info.height;

            if (height < childHeight)
            {
                height = childHeight;
            }

            m.children.push_back(std::move(info.mesh));
        }

        return InnerUnitMeshInfo{std::move(m), height};
    }

    MeshService::UnitMeshInfo MeshService::loadUnitMesh(const std::string& name, unsigned int teamColor)
    {
        auto bytes = vfs->readFile("objects3d/" + name + ".3do");
        if (!bytes)
        {
            throw std::runtime_error("Failed to load object bytes: " + name);
        }

        boost::interprocess::bufferstream s(bytes->data(), bytes->size());
        auto objects = parse3doObjects(s, s.tellg());
        assert(objects.size() == 1);
        auto selectionMesh = selectionMeshFrom3do(objects.front());
        auto unitMesh = unitMeshFrom3do(objects.front(), teamColor);
        return UnitMeshInfo{std::move(unitMesh.mesh), std::move(selectionMesh), unitMesh.height};
    }

    SharedTextureHandle MeshService::getMeshTextureAtlas()
    {
        return atlas;
    }

    Rectangle2f MeshService::getTextureRegion(const std::string& name, unsigned int teamColor)
    {
        auto attrsIt = textureAttributesMap.find(name);
        if (attrsIt == textureAttributesMap.end())
        {
            throw std::runtime_error("Texture attributes not found for texture: " + name);
        }

        const auto& attrs = attrsIt->second;
        unsigned int frameNumber = 0;
        if (attrs.isTeamDependent)
        {
            frameNumber = teamColor;
        }

        FrameId frameId(name, frameNumber);
        auto it = atlasMap.find(frameId);
        if (it == atlasMap.end())
        {
            throw std::runtime_error("Texture not found in atlas: " + name);
        }

        return it->second;
    }

    SelectionMesh MeshService::selectionMeshFrom3do(const _3do::Object& o)
    {
        assert(!!o.selectionPrimitiveIndex);
        auto index = *o.selectionPrimitiveIndex;
        auto p = o.primitives.at(index);

        assert(p.vertices.size() == 4);
        Vector3f offset(vertexToVector(_3do::Vertex(o.x, o.y, o.z)));

        auto a = offset + vertexToVector(o.vertices[p.vertices[0]]);
        auto b = offset + vertexToVector(o.vertices[p.vertices[1]]);
        auto c = offset + vertexToVector(o.vertices[p.vertices[2]]);
        auto d = offset + vertexToVector(o.vertices[p.vertices[3]]);

        auto collisionMesh = CollisionMesh::fromQuad(a, b, c, d);
        auto selectionMesh = createSelectionMesh(a, b, c, d);

        return SelectionMesh{std::move(collisionMesh), std::move(selectionMesh)};
    }

    GlMesh MeshService::createSelectionMesh(const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d)
    {
        const Vector3f color(0.325f, 0.875f, 0.310f);

        std::vector<GlColoredVertex> buffer{
            {a, color},
            {b, color},
            {c, color},
            {d, color}};

        return graphics->createColoredMesh(buffer, GL_STATIC_DRAW);
    }

    Vector3f getNormal(const Mesh::Triangle& t)
    {
        auto v1 = t.b.position - t.a.position;
        auto v2 = t.c.position - t.a.position;
        return v1.cross(v2).normalizedOr(Vector3f(1.0f, 0.0f, 0.0f));
    }

    ShaderMesh MeshService::convertMesh(const Mesh& mesh)
    {
        std::vector<GlTexturedNormalVertex> texturedVerticesBuffer;
        texturedVerticesBuffer.reserve(mesh.faces.size() * 3);

        for (const auto& t : mesh.faces)
        {
            auto normal = getNormal(t);
            texturedVerticesBuffer.emplace_back(t.a.position, t.a.textureCoord, normal);
            texturedVerticesBuffer.emplace_back(t.b.position, t.b.textureCoord, normal);
            texturedVerticesBuffer.emplace_back(t.c.position, t.c.textureCoord, normal);
        }

        auto texturedMesh = graphics->createTexturedNormalMesh(texturedVerticesBuffer, GL_STATIC_DRAW);

        std::vector<GlColoredNormalVertex> coloredVerticesBuffer;
        for (const auto& t : mesh.colorFaces)
        {
            auto color = Vector3f(t.color.r, t.color.g, t.color.b) / 255.0f;
            auto normal = getNormal(t);

            coloredVerticesBuffer.emplace_back(t.a.position, color, normal);
            coloredVerticesBuffer.emplace_back(t.b.position, color, normal);
            coloredVerticesBuffer.emplace_back(t.c.position, color, normal);
        }

        auto coloredMesh = graphics->createColoredNormalMesh(coloredVerticesBuffer, GL_STATIC_DRAW);

        return ShaderMesh(mesh.texture, std::move(texturedMesh), std::move(coloredMesh));
    }
}
