#pragma once

#include <optional>
#include <rwe/SelectionMesh.h>
#include <rwe/UnitPieceMeshInfo.h>
#include <rwe/io/_3do/_3do.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/sim/UnitPieceDefinition.h>
#include <string>
#include <vector>

namespace rwe
{
    struct TextureRegionInfo
    {
        bool isTeamColor;
        Rectangle2f region;
    };

    TextureRegionInfo getTextureRegion(
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::string& name);

    Vector2f getColorTexturePoint(
        const std::vector<Vector2f>& atlasColorMap,
        unsigned int colorIndex);

    void extractMeshes(
        GraphicsContext& graphics,
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::vector<Vector2f>& atlasColorMap,
        const _3do::Object& o,
        std::vector<std::pair<std::string, UnitPieceMeshInfo>>& v);

    Mesh meshFrom3do(
        const std::unordered_map<std::string, Rectangle2f>& atlasMap,
        const std::unordered_map<std::string, Rectangle2f>& teamAtlasMap,
        const std::vector<Vector2f>& atlasColorMap,
        const _3do::Object& o);

    SelectionMesh selectionMeshFrom3do(GraphicsContext& graphics, const _3do::Object& o);

    GlMesh createSelectionMesh(GraphicsContext& graphics, const Vector3f& a, const Vector3f& b, const Vector3f& c, const Vector3f& d);

    ShaderMesh convertMesh(GraphicsContext& graphics, const Mesh& mesh);

    std::vector<UnitPieceDefinition> unitMeshFrom3do(const _3do::Object& o, const std::optional<std::string>& parent);
    std::vector<UnitPieceDefinition> unitMeshFrom3do(const _3do::Object& o);

    Vector3f vertexToVector(const _3do::Vertex& v);
}
