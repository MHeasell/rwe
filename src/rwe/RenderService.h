#ifndef RWE_RENDERSERVICE_H
#define RWE_RENDERSERVICE_H

#include "GraphicsContext.h"
#include "OccupiedGrid.h"
#include "ShaderService.h"
#include "Unit.h"

namespace rwe
{
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
        void renderFlatFeatures(const MapTerrain& terrain);
        void renderStandingFeatures(const MapTerrain& terrain);

        void drawFeature(const MapFeature& feature);

        void drawStandingSprite(const Vector3f& position, const Sprite& sprite);
        void drawStandingSprite(const Vector3f& position, const Sprite& sprite, float alpha);

        void drawMapTerrain(const MapTerrain& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);


    private:
        GlMesh createTemporaryLinesMesh(const std::vector<Line3f>& lines);
        GlMesh createTemporaryTriMesh(const std::vector<Triangle3f>& tris);
    };
}

#endif
