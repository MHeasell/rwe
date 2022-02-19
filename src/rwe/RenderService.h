#pragma once

#include <boost/iterator/filter_iterator.hpp>
#include <rwe/FlashEffect.h>
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
        UnitDatabase* const unitDatabase;

        const Matrix4f* const viewProjectionMatrix;

        const MeshDatabase* const meshDatabase;
        const SharedTextureHandle* const unitTextureAtlas;
        const std::vector<SharedTextureHandle>* const unitTeamTextureAtlases;

    public:
        RenderService(
            GraphicsContext* graphics,
            ShaderService* shaders,
            UnitDatabase* unitDatabase,
            const Matrix4f* viewProjectionMatrix,
            const MeshDatabase* meshDatabase,
            const SharedTextureHandle* unitTextureAtlas,
            const std::vector<SharedTextureHandle>* unitTeamTextureAtlases);

        void drawProjectileUnitMesh(const std::string& objectName, const Matrix4f& modelMatrix, float seaLevel, PlayerColorIndex playerColorIndex, bool shaded);
        void drawSelectionRect(const Unit& unit, float frac);

        void drawMapTerrain(const MapTerrainGraphics& terrain, const Vector3f& cameraPosition, float viewportWidth, float viewportHeight);

        void drawMapTerrain(const MapTerrainGraphics& terrain, unsigned int x, unsigned int y, unsigned int width, unsigned int height);

        void fillScreen(float r, float g, float b, float a);

        void drawProjectiles(const VectorMap<Projectile, ProjectileIdTag>& projectiles, float seaLevel, GameTime currentTime, float frac);

        void drawFlashes(GameTime currentTime, const std::vector<FlashEffect>& flashes);

        void drawBatch(const ColoredMeshBatch& batch, const Matrix4f& vpMatrix);

        void drawUnitMeshBatch(const UnitMeshBatch& batch, float seaLevel, float time);

        void drawUnitShadowMeshBatch(const UnitShadowMeshBatch& batch);

        void drawSpriteBatch(const SpriteBatch& batch);

    private:
        void drawShaderMesh(const ShaderMesh& mesh, const Matrix4f& matrix, float seaLevel, bool shaded, PlayerColorIndex playerColorIndex);
    };
}
