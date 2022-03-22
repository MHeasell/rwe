#include "mesh_util.h"
#include <rwe/fixed_point.h>

namespace rwe
{
    Vector3f getNormal(const Mesh::Triangle& t)
    {
        auto v1 = t.b.position - t.a.position;
        auto v2 = t.c.position - t.a.position;
        return v1.cross(v2).normalizedOr(Vector3f(1.0f, 0.0f, 0.0f));
    }

    TextureRegionInfo getTextureRegion(
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::string& name)
    {
        auto it = atlasMap.find(name);
        if (it != atlasMap.end())
        {
            return TextureRegionInfo{false, it->second};
        }

        auto it2 = teamAtlasMap.find(name);
        if (it2 != teamAtlasMap.end())
        {
            return TextureRegionInfo{true, it2->second};
        }

        // Some unit models in the wild (e.g. the ARM commander in TA:Zero)
        // contain references to textures that don't exist,
        // so we cannot simply throw here.
        // TODO: consider returning an optional and making the caller decide what to do
        return {false, Rectangle2f(0, 0, 0, 0)};
    }

    Vector2f getColorTexturePoint(
        const std::vector<Vector2f>& atlasColorMap,
        unsigned int colorIndex)
    {
        assert(colorIndex < atlasColorMap.size());
        return atlasColorMap[colorIndex];
    }

    void extractMeshes(
        GraphicsContext& graphics,
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::vector<Vector2f>& atlasColorMap,
        const _3do::Object& o,
        std::vector<std::pair<std::string, UnitPieceMeshInfo>>& v)
    {
        auto firstVertex = o.vertices.size() > 0 ? vertexToVector(o.vertices[0]) : Vector3f(0.0f, 0.0f, 0.0f);
        auto secondVertex = o.vertices.size() > 1 ? vertexToVector(o.vertices[1]) : Vector3f(1.0f, 0.0f, 0.0f);
        auto mesh = meshFrom3do(atlasMap, teamAtlasMap, atlasColorMap, o);
        auto shaderMesh = convertMesh(graphics, mesh);

        v.push_back(std::make_pair(o.name, UnitPieceMeshInfo{std::make_shared<ShaderMesh>(std::move(shaderMesh)), firstVertex, secondVertex}));

        for (const auto& c : o.children)
        {
            extractMeshes(graphics, atlasMap, teamAtlasMap, atlasColorMap, c, v);
        }
    }

    Mesh meshFrom3do(
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::vector<Vector2f>& atlasColorMap,
        const _3do::Object& o)
    {
        Mesh m;

        for (const auto& p : o.primitives)
        {
            // handle textured quads
            if (p.vertices.size() == 4 && p.textureName)
            {
                auto textureBounds = getTextureRegion(atlasMap, teamAtlasMap, *(p.textureName));

                Mesh::Triangle t0(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.region.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[1]]), textureBounds.region.topRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.region.topLeft()));

                Mesh::Triangle t1(
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[3]]), textureBounds.region.bottomLeft()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[2]]), textureBounds.region.bottomRight()),
                    Mesh::Vertex(vertexToVector(o.vertices[p.vertices[0]]), textureBounds.region.topLeft()));

                if (textureBounds.isTeamColor)
                {
                    m.teamFaces.push_back(t0);
                    m.teamFaces.push_back(t1);
                }
                else
                {
                    m.faces.push_back(t0);
                    m.faces.push_back(t1);
                }

                continue;
            }

            // handle other polygon types
            if (p.vertices.size() >= 3 && p.colorIndex)
            {
                auto texturePosition = getColorTexturePoint(atlasColorMap, *p.colorIndex);
                const auto& first = vertexToVector(o.vertices[p.vertices.front()]);
                for (Index i = getSize(p.vertices) - 1; i >= 2; --i)
                {
                    const auto& second = vertexToVector(o.vertices[p.vertices[i]]);
                    const auto& third = vertexToVector(o.vertices[p.vertices[i - 1]]);
                    Mesh::Triangle t(
                        Mesh::Vertex(first, texturePosition),
                        Mesh::Vertex(second, texturePosition),
                        Mesh::Vertex(third, texturePosition));
                    m.faces.push_back(t);
                }

                continue;
            }

            // just ignore degenerate faces (0, 1 or 2 vertices)
        }

        return m;
    }

    SelectionMesh selectionMeshFrom3do(GraphicsContext& graphics, const _3do::Object& o)
    {
        auto index = o.selectionPrimitiveIndex.value_or(0u);
        auto p = o.primitives.at(index);

        assert(p.vertices.size() == 4);
        Vector3f offset(vertexToVector(_3do::Vertex(o.x, o.y, o.z)));

        auto a = offset + vertexToVector(o.vertices[p.vertices[0]]);
        auto b = offset + vertexToVector(o.vertices[p.vertices[1]]);
        auto c = offset + vertexToVector(o.vertices[p.vertices[2]]);
        auto d = offset + vertexToVector(o.vertices[p.vertices[3]]);

        auto collisionMesh = CollisionMesh::fromQuad(a, b, c, d);
        auto selectionMesh = createSelectionMesh(graphics, a, b, c, d);

        return SelectionMesh{std::move(collisionMesh), std::move(selectionMesh)};
    }

    GlMesh createSelectionMesh(GraphicsContext& graphics, const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d)
    {
        const Vector3f color(0.325f, 0.875f, 0.310f);

        std::vector<GlColoredVertex> buffer{
            {a, color},
            {b, color},
            {c, color},
            {d, color}};

        return graphics.createColoredMesh(buffer, GL_STATIC_DRAW);
    }

    ShaderMesh convertMesh(GraphicsContext& graphics, const Mesh& mesh)
    {
        std::optional<GlMesh> texturedMesh;
        if (!mesh.faces.empty())
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

            texturedMesh = graphics.createTexturedNormalMesh(texturedVerticesBuffer, GL_STATIC_DRAW);
        }

        std::optional<GlMesh> teamTexturedMesh;
        if (!mesh.teamFaces.empty())
        {

            std::vector<GlTexturedNormalVertex> teamTexturedVerticesBuffer;
            teamTexturedVerticesBuffer.reserve(mesh.teamFaces.size() * 3);
            for (const auto& t : mesh.teamFaces)
            {
                auto normal = getNormal(t);
                teamTexturedVerticesBuffer.emplace_back(t.a.position, t.a.textureCoord, normal);
                teamTexturedVerticesBuffer.emplace_back(t.b.position, t.b.textureCoord, normal);
                teamTexturedVerticesBuffer.emplace_back(t.c.position, t.c.textureCoord, normal);
            }

            teamTexturedMesh = graphics.createTexturedNormalMesh(teamTexturedVerticesBuffer, GL_STATIC_DRAW);
        }

        return ShaderMesh(std::move(texturedMesh), std::move(teamTexturedMesh));
    }

    std::vector<UnitPieceDefinition> unitMeshFrom3do(const _3do::Object& o, const std::optional<std::string>& parent)
    {
        UnitPieceDefinition m;
        m.origin = Vector3x<SimScalar>(
            simScalarFromFixed(o.x),
            simScalarFromFixed(o.y),
            -simScalarFromFixed(o.z)); // flip to convert from left-handed to right-handed
        m.name = o.name;
        m.parent = parent;

        std::vector<UnitPieceDefinition> pieces{m};

        for (const auto& c : o.children)
        {
            auto childPieces = unitMeshFrom3do(c, o.name);
            for (auto& p : childPieces)
            {
                pieces.push_back(p);
            }
        }

        return pieces;
    }

    std::vector<UnitPieceDefinition> unitMeshFrom3do(const _3do::Object& o)
    {
        return unitMeshFrom3do(o, std::nullopt);
    }

    Vector3f vertexToVector(const _3do::Vertex& v)
    {
        return Vector3f(
            fromFixedPoint(v.x),
            fromFixedPoint(v.y),
            fromFixedPoint(-v.z)); // flip to convert from left-handed to right-handed
    }
}
