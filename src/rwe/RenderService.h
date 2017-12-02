#ifndef RWE_RENDERSERVICE_H
#define RWE_RENDERSERVICE_H

#include "GraphicsContext.h"
#include "OccupiedGrid.h"
#include "ShaderService.h"
#include "Unit.h"
#include <vector>
#include <boost/iterator/filter_iterator.hpp>

namespace rwe
{
    struct IsFeatureBlocking
    {
        bool operator()(const MapFeature& f) const
        {
            return f.isBlocking();
        }
    };

    struct IsFeatureNotBlocking
    {
        bool operator()(const MapFeature& f) const
        {
            return !f.isBlocking();
        }
    };

    class RenderService
    {
    private:
        GraphicsContext* graphics;
        ShaderService* shaders;

        CabinetCamera camera;

    public:
        RenderService(
            GraphicsContext* graphics,
            ShaderService* shaders,
            const CabinetCamera& camera);

        CabinetCamera& getCamera();
        const CabinetCamera& getCamera() const;

        void renderUnit(const Unit& unit, float seaLevel);
        void renderUnitShadow(const Unit& unit, float groundHeight);
        void renderUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, float seaLevel);
        void renderSelectionRect(const Unit& unit);
        void renderOccupiedGrid(const MapTerrain& terrain, const OccupiedGrid& occupiedGrid);

        void renderMapTerrain(const MapTerrain& terrain);
        void drawFlatFeatures(const MapTerrain& terrain);
        void drawFlatFeatureShadows(const MapTerrain& terrain);
        void drawStandingFeatures(const MapTerrain& terrain);
        void drawStandingFeatureShadows(const MapTerrain& terrain);

        void drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

        void drawUnitShadows(const MapTerrain& terrain, const std::vector<Unit>& units);

        void fillScreen(float r, float g, float b, float a);

    private:
        GlMesh createTemporaryLinesMesh(const std::vector<Line3f>& lines);
        GlMesh createTemporaryTriMesh(const std::vector<Triangle3f>& tris);

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
            auto fBegin = boost::make_filter_iterator<IsFeatureNotBlocking>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureNotBlocking>(end, end);

            drawFeaturesInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawStandingFeaturesInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureBlocking>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureBlocking>(end, end);

            drawFeaturesInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawStandingFeatureShadowsInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureBlocking>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureBlocking>(end, end);

            drawFeatureShadowsInternal(fBegin, fEnd);
        }

        template <typename It>
        void drawFlatFeatureShadowsInternal(It begin, It end)
        {
            auto fBegin = boost::make_filter_iterator<IsFeatureBlocking>(begin, end);
            auto fEnd = boost::make_filter_iterator<IsFeatureBlocking>(end, end);

            drawFeatureShadowsInternal(fBegin, fEnd);
        }
    };
}

#endif
