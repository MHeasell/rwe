#pragma once

#include <boost/iterator/filter_iterator.hpp>
#include <rwe/MapTerrainGraphics.h>
#include <rwe/MeshDatabase.h>
#include <rwe/PlayerColorIndex.h>
#include <rwe/ShaderService.h>
#include <rwe/UnitDatabase.h>
#include <rwe/VectorMap.h>
#include <rwe/pathfinding/AStarPathFinder.h>
#include <rwe/pathfinding/OctileDistance.h>
#include <rwe/pathfinding/PathCost.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/sim/Explosion.h>
#include <rwe/sim/GameTime.h>
#include <rwe/sim/OccupiedGrid.h>
#include <rwe/sim/Projectile.h>
#include <rwe/sim/ProjectileId.h>
#include <rwe/sim/Unit.h>
#include <vector>

namespace rwe
{
    struct IsFeatureStanding
    {
        bool operator()(const MapFeature& f) const
        {
            return f.isStanding();
        }
    };

    struct IsFeatureNotStanding
    {
        bool operator()(const MapFeature& f) const
        {
            return !f.isStanding();
        }
    };

    struct ColoredMeshBatch
    {
        std::vector<GlColoredVertex> lines;
        std::vector<GlColoredVertex> triangles;
    };

    struct UnitTextureMeshRenderInfo
    {
        const GlMesh* mesh;
        Matrix4f modelMatrix;
        Matrix4f mvpMatrix;
        bool shaded;
        TextureIdentifier texture;
    };

    struct UnitBuildingMeshRenderInfo
    {
        const GlMesh* mesh;
        Matrix4f modelMatrix;
        Matrix4f mvpMatrix;
        bool shaded;
        TextureIdentifier texture;
        float percentComplete;
        float unitY;
    };

    struct UnitMeshBatch
    {
        std::vector<UnitTextureMeshRenderInfo> meshes;
        std::vector<UnitBuildingMeshRenderInfo> buildingMeshes;
    };

    class RenderService
    {
    private:
        GraphicsContext* graphics;
        ShaderService* shaders;
        UnitDatabase* const unitDatabase;

        const CabinetCamera* const camera;

        const MeshDatabase* const meshDatabase;
        const SharedTextureHandle* const unitTextureAtlas;
        const std::vector<SharedTextureHandle>* const unitTeamTextureAtlases;

    public:
        RenderService(
            GraphicsContext* graphics,
            ShaderService* shaders,
            const MeshDatabase* meshDatabase,
            UnitDatabase* unitDatabase,
            const CabinetCamera* camera,
            const SharedTextureHandle* unitTextureAtlas,
            const std::vector<SharedTextureHandle>* unitTeamTextureAtlases);

        void drawUnitShadow(const Unit& unit, float groundHeight, float frac);
        void drawUnitMeshShadow(const std::string& objectName, const std::vector<UnitMesh>& meshes, const Matrix4f& modelMatrix, float groundHeight, float frac);
        void drawProjectileUnitMesh(const std::string& objectName, const Matrix4f& modelMatrix, float seaLevel, PlayerColorIndex playerColorIndex, bool shaded);
        void drawFeatureUnitMeshShadow(const std::string& objectName, const Matrix4f& modelMatrix, float groundHeight);
        void drawSelectionRect(const Unit& unit, float frac);
        void drawNanolatheLine(const Vector3f& start, const Vector3f& end);

        void drawMapTerrain(const MapTerrainGraphics& terrain);

        template <typename Range>
        void drawFlatFeatures(const Range& features)
        {
            drawFlatFeaturesInternal(features.begin(), features.end());
        }

        template <typename Range>
        void drawFlatFeatureShadows(const Range& features)
        {
            drawFlatFeatureShadowsInternal(features.begin(), features.end());
        }

        template <typename Range>
        void drawStandingFeatures(const Range& features)
        {
            drawStandingFeaturesInternal(features.begin(), features.end());
        }

        template <typename Range>
        void drawStandingFeatureShadows(const Range& features)
        {
            drawStandingFeatureShadowsInternal(features.begin(), features.end());
        }

        void drawMapTerrain(const MapTerrainGraphics& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

        template <typename Range>
        void drawUnitShadows(const MapTerrain& terrain, const Range& units, float frac, SimScalar seaLevel)
        {
            graphics->enableStencilBuffer();
            graphics->clearStencilBuffer();
            graphics->useStencilBufferForWrites();
            graphics->disableColorBuffer();

            for (const Unit& unit : units)
            {
                auto position = lerp(simVectorToFloat(unit.previousPosition), simVectorToFloat(unit.position), frac);
                auto groundHeight = terrain.getHeightAt(floatToSimScalar(position.x), floatToSimScalar(position.z));
                if (unit.floater || unit.canHover)
                {
                    groundHeight = rweMax(groundHeight, seaLevel);
                }
                drawUnitShadow(unit, simScalarToFloat(groundHeight), frac);
            }

            graphics->useStencilBufferAsMask();
            graphics->enableColorBuffer();

            fillScreen(0.0f, 0.0f, 0.0f, 0.5f);

            graphics->enableColorBuffer();
            graphics->disableStencilBuffer();
        }

        template <typename Range>
        void drawFeatureMeshShadows(const MapTerrain& terrain, const Range& features, SimScalar seaLevel)
        {
            graphics->enableStencilBuffer();
            graphics->clearStencilBuffer();
            graphics->useStencilBufferForWrites();
            graphics->disableColorBuffer();

            for (const MapFeature& feature : features)
            {
                auto objectInfo = std::get_if<FeatureObjectInfo>(&feature.renderInfo);
                if (objectInfo == nullptr)
                {
                    continue;
                }

                const auto& position = feature.position;
                auto groundHeight = terrain.getHeightAt(position.x, position.z);
                if (position.y >= seaLevel && groundHeight < seaLevel)
                {
                    groundHeight = seaLevel;
                }

                auto matrix = Matrix4f::translation(simVectorToFloat(position)) * Matrix4f::rotationY(0.0f, -1.0f);

                drawFeatureUnitMeshShadow(objectInfo->objectName, matrix, simScalarToFloat(groundHeight));
            }

            graphics->useStencilBufferAsMask();
            graphics->enableColorBuffer();

            fillScreen(0.0f, 0.0f, 0.0f, 0.5f);

            graphics->enableColorBuffer();
            graphics->disableStencilBuffer();
        }

        void fillScreen(float r, float g, float b, float a);

        void drawProjectiles(const VectorMap<Projectile, ProjectileIdTag>& projectiles, float seaLevel, GameTime currentTime, float frac);

        void drawExplosions(GameTime currentTime, const std::vector<Explosion>& explosions);

        void drawBatch(const ColoredMeshBatch& batch, const Matrix4f& vpMatrix);

        void drawUnitMeshBatch(const UnitMeshBatch& batch, float seaLevel, float time);

    private:
        void drawShaderMesh(const ShaderMesh& mesh, const Matrix4f& matrix, float seaLevel, bool shaded, PlayerColorIndex playerColorIndex);

        void drawShaderMeshShadow(const ShaderMesh& mesh, const Matrix4f& matrix, float groundHeight);

        void drawBuildingShaderMesh(const ShaderMesh& mesh, const Matrix4f& matrix, float seaLevel, bool shaded, float percentComplete, float unitY, float time, PlayerColorIndex playerColorIndex);

        GlMesh createTemporaryLinesMesh(const std::vector<Line3f>& lines);

        GlMesh createTemporaryLinesMesh(const std::vector<Line3f>& lines, const Vector3f& color);

        GlMesh createTemporaryTriMesh(const std::vector<Triangle3f>& tris);
        GlMesh createTemporaryTriMesh(const std::vector<Triangle3f>& tris, const Vector3f& color);

        void drawFeatureShadowInternal(const MapFeature& feature);
        void drawFeatureInternal(const MapFeature& feature);

        template <typename It>
        void drawFeatureShadowsInternal(It begin, It end)
        {
            graphics->bindShader(shaders->basicTexture.handle.get());
            for (auto it = begin; it != end; ++it)
            {
                const MapFeature& feature = *it;
                drawFeatureShadowInternal(feature);
            }
        }

        template <typename It>
        void drawFeaturesInternal(It begin, It end)
        {
            graphics->bindShader(shaders->basicTexture.handle.get());
            for (auto it = begin; it != end; ++it)
            {
                const MapFeature& feature = *it;
                drawFeatureInternal(feature);
            }
        }

        template <typename It>
        void drawFlatFeaturesInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureNotStanding>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureNotStanding>(end, end);

            drawFeaturesInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawStandingFeaturesInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureStanding>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureStanding>(end, end);

            drawFeaturesInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawStandingFeatureShadowsInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureStanding>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureStanding>(end, end);

            drawFeatureShadowsInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawFlatFeatureShadowsInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureNotStanding>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureNotStanding>(end, end);

            drawFeatureShadowsInternal(fBegin, fEnd);
        }
    };
}
