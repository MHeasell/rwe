#include "RenderService.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    class IsOccupiedVisitor : public boost::static_visitor<bool>
    {
    public:
        bool operator()(const OccupiedNone&) const
        {
            return false;
        }

        bool operator()(const OccupiedUnit&) const
        {
            return true;
        }

        bool operator()(const OccupiedFeature&) const
        {
            return true;
        }
    };

    RenderService::RenderService(
        GraphicsContext* graphics,
        const SharedShaderProgramHandle& unitTextureShader,
        const SharedShaderProgramHandle& unitColorShader,
        const SharedShaderProgramHandle& basicColorShader)
        : graphics(graphics),
          unitTextureShader(unitTextureShader),
          unitColorShader(unitColorShader),
          basicColorShader(basicColorShader)
    {
    }

    void
    RenderService::renderSelectionRect(const Unit& unit, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix)
    {
        // try to ensure that the selection rectangle vertices
        // are aligned with the middle of pixels,
        // to prevent discontinuities in the drawn lines.
        Vector3f snappedPosition(
            snapToInterval(unit.position.x, 1.0f) + 0.5f,
            snapToInterval(unit.position.y, 2.0f),
            snapToInterval(unit.position.z, 1.0f) + 0.5f);

        auto matrix = Matrix4f::translation(snappedPosition) * Matrix4f::rotationY(unit.rotation);

        graphics->drawWireframeSelectionMesh(unit.selectionMesh.visualMesh, matrix, viewMatrix, projectionMatrix, basicColorShader.get());
    }

    void RenderService::renderUnit(const Unit& unit, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix, float seaLevel)
    {
        auto matrix = Matrix4f::translation(unit.position) * Matrix4f::rotationY(unit.rotation);
        renderUnitMesh(unit.mesh, matrix, viewMatrix, projectionMatrix, seaLevel);
    }

    void RenderService::renderUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix, float seaLevel)
    {
        Vector3f testRotation(-mesh.rotation.x, mesh.rotation.y, mesh.rotation.z);
        auto matrix = modelMatrix * Matrix4f::translation(mesh.origin) * Matrix4f::rotationXYZ(testRotation) * Matrix4f::translation(mesh.offset);

        if (mesh.visible)
        {
            graphics->drawShaderMesh(*(mesh.mesh), unitTextureShader.get(), unitColorShader.get(), matrix, viewMatrix, projectionMatrix, seaLevel);
        }

        for (const auto& c : mesh.children)
        {
            renderUnitMesh(c, matrix, viewMatrix, projectionMatrix, seaLevel);
        }
    }

    void RenderService::renderOccupiedGrid(const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, const CabinetCamera& camera, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix)
    {
        auto halfWidth = camera.getWidth() / 2.0f;
        auto halfHeight = camera.getHeight() / 2.0f;
        auto left = camera.getPosition().x - halfWidth;
        auto top = camera.getPosition().z - halfHeight;
        auto right = camera.getPosition().x + halfWidth;
        auto bottom = camera.getPosition().z + halfHeight;

        assert(left < right);
        assert(top < bottom);

        assert(terrain.getHeightMap().getWidth() >= 2);
        assert(terrain.getHeightMap().getHeight() >= 2);

        auto topLeftCell = terrain.worldToHeightmapCoordinate(Vector3f(left, 0.0f, top));
        topLeftCell.x = std::clamp(topLeftCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        topLeftCell.y = std::clamp(topLeftCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        auto bottomRightCell = terrain.worldToHeightmapCoordinate(Vector3f(right, 0.0f, bottom));
        bottomRightCell.y += 7; // compensate for height
        bottomRightCell.x = std::clamp(bottomRightCell.x, 0, static_cast<int>(terrain.getHeightMap().getWidth() - 2));
        bottomRightCell.y = std::clamp(bottomRightCell.y, 0, static_cast<int>(terrain.getHeightMap().getHeight() - 2));

        assert(topLeftCell.x <= bottomRightCell.x);
        assert(topLeftCell.y <= bottomRightCell.y);

        std::vector<Line3f> lines;

        std::vector<Triangle3f> tris;

        for (int y = topLeftCell.y; y <= bottomRightCell.y; ++y)
        {
            for (int x = topLeftCell.x; x <= bottomRightCell.x; ++x)
            {
                auto pos = terrain.heightmapIndexToWorldCorner(x, y);
                pos.y = terrain.getHeightMap().get(x, y);

                auto rightPos = terrain.heightmapIndexToWorldCorner(x + 1, y);
                rightPos.y = terrain.getHeightMap().get(x + 1, y);

                auto downPos = terrain.heightmapIndexToWorldCorner(x, y + 1);
                downPos.y = terrain.getHeightMap().get(x, y + 1);

                lines.emplace_back(pos, rightPos);
                lines.emplace_back(pos, downPos);

                if (boost::apply_visitor(IsOccupiedVisitor(), occupiedGrid.grid.get(x, y)))
                {
                    auto downRightPos = terrain.heightmapIndexToWorldCorner(x + 1, y + 1);
                    downRightPos.y = terrain.getHeightMap().get(x + 1, y + 1);

                    tris.emplace_back(pos, downPos, downRightPos);
                    tris.emplace_back(pos, downRightPos, rightPos);
                }
            }
        }

        auto mesh = createTemporaryLinesMesh(lines);
        graphics->drawLinesMesh(
            mesh,
            Matrix4f::identity(),
            viewMatrix,
            projectionMatrix,
            basicColorShader.get());

        auto triMesh = createTemporaryTriMesh(tris);
        graphics->drawTrisMesh(
            triMesh,
            Matrix4f::identity(),
            viewMatrix,
            projectionMatrix,
            basicColorShader.get());
    }

    GlMesh RenderService::createTemporaryLinesMesh(const std::vector<Line3f>& lines)
    {
        std::vector<GlColoredVertex> buffer;
        buffer.reserve(lines.size() * 2); // 2 verts per line

        Vector3f white(1.0f, 1.0f, 1.0f);

        for (const auto& l : lines)
        {
            buffer.emplace_back(l.start, white);
            buffer.emplace_back(l.end, white);
        }

        return graphics->createColoredMesh(buffer, GL_STREAM_DRAW);
    }

    GlMesh RenderService::createTemporaryTriMesh(const std::vector<Triangle3f>& tris)
    {
        std::vector<GlColoredVertex> buffer;
        buffer.reserve(tris.size() * 3); // 3 verts per triangle

        Vector3f white(1.0f, 1.0f, 1.0f);

        for (const auto& l : tris)
        {
            buffer.emplace_back(l.a, white);
            buffer.emplace_back(l.b, white);
            buffer.emplace_back(l.c, white);
        }

        return graphics->createColoredMesh(buffer, GL_STREAM_DRAW);
    }

}
