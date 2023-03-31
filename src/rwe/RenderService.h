#pragma once

#include <rwe/ShaderService.h>
#include <rwe/game/FlashEffect.h>
#include <rwe/game/MapTerrainGraphics.h>
#include <rwe/render/GraphicsContext.h>
#include <rwe/sim/GameTime.h>
#include <vector>

namespace rwe
{
    struct ColoredMeshBatch
    {
        std::vector<GlColoredVertex> lines;
        std::vector<GlColoredVertex> triangles;
    };

    struct ColoredMeshRenderInfo
    {
        const GlMesh* mesh;
        Matrix4f mvpMatrix;
    };

    struct ColoredMeshesBatch
    {
        std::vector<ColoredMeshRenderInfo> meshes;
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

    struct UnitTextureShadowMeshRenderInfo
    {
        const GlMesh* mesh;
        Matrix4f modelMatrix;
        Matrix4f vpMatrix;
        TextureIdentifier texture;
        float groundHeight;
    };

    struct UnitMeshBatch
    {
        std::vector<UnitTextureMeshRenderInfo> meshes;
        std::vector<UnitBuildingMeshRenderInfo> buildingMeshes;
    };

    struct UnitShadowMeshBatch
    {
        std::vector<UnitTextureShadowMeshRenderInfo> meshes;
    };

    struct SpriteRenderInfo
    {
        const Sprite* sprite;
        Matrix4f mvpMatrix;
        bool translucent;
    };

    struct SpriteBatch
    {
        std::vector<SpriteRenderInfo> sprites;
    };

    class RenderService
    {
    private:
        GraphicsContext* graphics;
        ShaderService* shaders;

        const Matrix4f* const viewProjectionMatrix;

    public:
        RenderService(
            GraphicsContext* graphics,
            ShaderService* shaders,
            const Matrix4f* viewProjectionMatrix);

        void drawMapTerrain(const MapTerrainGraphics& terrain, const Vector3f& cameraPosition, float viewportWidth, float viewportHeight);

        void drawMapTerrain(const MapTerrainGraphics& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

        void fillScreen(float r, float g, float b, float a);

        void drawFlashes(GameTime currentTime, const std::vector<FlashEffect>& flashes);

        void drawBatch(const ColoredMeshBatch& batch, const Matrix4f& vpMatrix);

        void drawUnitMeshBatch(const UnitMeshBatch& batch, float seaLevel, float time);

        void drawUnitShadowMeshBatch(const UnitShadowMeshBatch& batch);

        void drawSpriteBatch(const SpriteBatch& batch);

        void drawLineLoopsBatch(const ColoredMeshesBatch& batch);
    };
}
