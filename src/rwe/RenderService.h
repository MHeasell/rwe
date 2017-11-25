#ifndef RWE_RENDERSERVICE_H
#define RWE_RENDERSERVICE_H

#include "GraphicsContext.h"
#include "OccupiedGrid.h"
#include "Unit.h"

namespace rwe
{
    class RenderService
    {
    private:
        GraphicsContext* graphics;

        SharedShaderProgramHandle unitTextureShader;
        SharedShaderProgramHandle unitColorShader;
        SharedShaderProgramHandle basicColorShader;

    public:
        RenderService(GraphicsContext* graphics, const SharedShaderProgramHandle& unitTextureShader, const SharedShaderProgramHandle& unitColorShader, const SharedShaderProgramHandle& basicColorShader);

        void renderUnit(const Unit& unit, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix, float seaLevel);
        void renderUnitMesh(const UnitMesh& mesh, const Matrix4f& modelMatrix, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix, float seaLevel);
        void renderSelectionRect(const Unit& unit, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix);
        void renderOccupiedGrid(const MapTerrain& terrain, const OccupiedGrid& occupiedGrid, const CabinetCamera& camera, const Matrix4f& viewMatrix, const Matrix4f& projectionMatrix);
    };
}

#endif
