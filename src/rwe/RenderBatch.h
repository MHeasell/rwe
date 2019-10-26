#pragma once

#include "GlMesh.h"
#include "TextureHandle.h"
#include <rwe/math/Matrix4f.h>
#include <vector>
namespace rwe
{
    struct SpriteRenderCommand
    {
        TextureIdentifier texture;
        Matrix4f mvpMatrix;
        GlMesh* mesh;
        float alpha;
    };

    struct MeshShadowRenderCommand
    {
        Matrix4f mvpMatrix;
        GlMesh* mesh;
    };

    struct MeshRenderCommand
    {
        TextureIdentifier texture;
        Matrix4f mvpMatrix;
        Matrix4f modelMatrix;
        GlMesh* mesh;
        bool shade;
    };

    struct RenderBatch
    {
        std::vector<MeshShadowRenderCommand> meshShadows;
        std::vector<SpriteRenderCommand> spriteShadows;
        std::vector<SpriteRenderCommand> sprites;
        std::vector<MeshRenderCommand> meshes;
    };
}
