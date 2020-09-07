#include "MeshService.h"
#include <boost/interprocess/streams/bufferstream.hpp>
#include <rwe/BoxTreeSplit.h>
#include <rwe/Gaf.h>
#include <rwe/Index.h>
#include <rwe/_3do.h>
#include <rwe/fixed_point.h>
#include <rwe/geometry/CollisionMesh.h>
#include <rwe/math/Vector3f.h>
#include <rwe/math/rwe_math.h>
#include <rwe/overloaded.h>
#include <rwe/rwe_string.h>
#include <rwe/vertex_height.h>

namespace rwe
{
    Vector3f vertexToVector(const _3do::Vertex& v)
    {
        return Vector3f(
            fromFixedPoint(v.x),
            fromFixedPoint(v.y),
            fromFixedPoint(-v.z)); // flip to convert from left-handed to right-handed
    }

    MeshService::MeshService(
        AbstractVirtualFileSystem* vfs,
        GraphicsContext* graphics,
        std::unordered_map<std::string, Rectangle2f>&& atlasMap,
        std::unordered_map<std::string, Rectangle2f>&& teamAtlasMap,
        std::vector<Vector2f>&& atlasColorMap)
        : vfs(vfs),
          graphics(graphics),
          atlasMap(std::move(atlasMap)),
          teamAtlasMap(std::move(teamAtlasMap)),
          atlasColorMap(std::move(atlasColorMap))
    {
    }

    Mesh MeshService::meshFrom3do(const _3do::Object& o)
    {
        Mesh m;

        for (const auto& p : o.primitives)
        {
            // handle textured quads
            if (p.vertices.size() == 4 && p.textureName)
            {
                auto textureBounds = getTextureRegion(*(p.textureName));

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
                auto texturePosition = getColorTexturePoint(*p.colorIndex);
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

    UnitPieceDefinition unitMeshFrom3do(const _3do::Object& o)
    {
        UnitPieceDefinition m;
        m.origin = Vector3x<SimScalar>(
            simScalarFromFixed(o.x),
            simScalarFromFixed(o.y),
            -simScalarFromFixed(o.z)); // flip to convert from left-handed to right-handed
        m.name = o.name;

        for (const auto& c : o.children)
        {
            m.children.push_back(unitMeshFrom3do(c));
        }

        return m;
    }

    void MeshService::extractMeshes(const _3do::Object& o, std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>>& v)
    {
        auto mesh = meshFrom3do(o);
        v.emplace_back(o.name, std::make_shared<ShaderMesh>(convertMesh(mesh)));

        for (const auto& c : o.children)
        {
            extractMeshes(c, v);
        }
    }

    MeshService::UnitMeshInfo MeshService::loadUnitMesh(const std::string& name)
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

        UnitModelDefinition d;
        d.rootPiece = unitMeshFrom3do(objects.front());
        d.height = simScalarFromFixed(findHighestVertex(objects.front()).y);

        std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>> meshes;
        extractMeshes(objects.front(), meshes);

        return UnitMeshInfo{std::move(d), std::move(meshes), std::move(selectionMesh)};
    }

    MeshService::ProjectileMeshInfo MeshService::loadProjectileMesh(const std::string& name)
    {
        auto bytes = vfs->readFile("objects3d/" + name + ".3do");
        if (!bytes)
        {
            throw std::runtime_error("Failed to load object bytes: " + name);
        }

        boost::interprocess::bufferstream s(bytes->data(), bytes->size());
        auto objects = parse3doObjects(s, s.tellg());
        assert(objects.size() == 1);

        UnitModelDefinition d;
        d.rootPiece = unitMeshFrom3do(objects.front());
        d.height = simScalarFromFixed(findHighestVertex(objects.front()).y);

        std::vector<std::pair<std::string, std::shared_ptr<ShaderMesh>>> meshes;
        extractMeshes(objects.front(), meshes);

        return ProjectileMeshInfo{std::move(d), std::move(meshes)};
    }

    MeshService::TextureRegionInfo MeshService::getTextureRegion(const std::string& name)
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

    Vector2f MeshService::getColorTexturePoint(unsigned int colorIndex)
    {
        assert(colorIndex < atlasColorMap.size());
        return atlasColorMap[colorIndex];
    }

    SelectionMesh MeshService::selectionMeshFrom3do(const _3do::Object& o)
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

        std::vector<GlTexturedNormalVertex> teamTexturedVerticesBuffer;
        teamTexturedVerticesBuffer.reserve(mesh.teamFaces.size() * 3);
        for (const auto& t : mesh.teamFaces)
        {
            auto normal = getNormal(t);
            teamTexturedVerticesBuffer.emplace_back(t.a.position, t.a.textureCoord, normal);
            teamTexturedVerticesBuffer.emplace_back(t.b.position, t.b.textureCoord, normal);
            teamTexturedVerticesBuffer.emplace_back(t.c.position, t.c.textureCoord, normal);
        }

        auto teamTexturedMesh = graphics->createTexturedNormalMesh(teamTexturedVerticesBuffer, GL_STATIC_DRAW);

        return ShaderMesh(std::move(texturedMesh), std::move(teamTexturedMesh));
    }
}
