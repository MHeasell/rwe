#pragma once

#include <optional>
#include <rwe/render/GlMesh.h>
#include <rwe/render/TextureHandle.h>
#include <rwe/render/VaoHandle.h>
#include <rwe/render/VboHandle.h>

namespace rwe
{
    struct ShaderMesh
    {
        std::optional<GlMesh> vertices;
        std::optional<GlMesh> teamVertices;

        ShaderMesh(
            std::optional<GlMesh>&& vertices,
            std::optional<GlMesh>&& teamVertices);
    };
}
