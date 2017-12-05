#ifndef RWE_RENDERSERVICE_H
#define RWE_RENDERSERVICE_H

#include "GraphicsContext.h"
#include "OccupiedGrid.h"
#include "ShaderService.h"
#include "Unit.h"
#include <boost/iterator/filter_iterator.hpp>
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

        void drawUnit(const Unit& unit, float seaLevel);
        void drawUnitShadow(const Unit& unit, float groundHeight);
        void drawUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, float seaLevel);
        void drawSelectionRect(const Unit& unit);
        void drawOccupiedGrid(const MapTerrain& terrain, const OccupiedGrid& occupiedGrid);

        void drawMapTerrain(const MapTerrain& terrain);
        void drawFlatFeatures(const std::vector<MapFeature>& features);
        void drawFlatFeatureShadows(const std::vector<MapFeature>& features);
        void drawStandingFeatures(const std::vector<MapFeature>& features);
        void drawStandingFeatureShadows(const std::vector<MapFeature>& features);

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

#endif
