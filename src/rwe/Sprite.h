#pragma once

#include <memory>
#include <rwe/GlMesh.h>
#include <rwe/TextureHandle.h>
#include <rwe/geometry/Rectangle2f.h>
#include <rwe/math/Matrix4f.h>

namespace rwe
{
    struct Sprite
    {
        Rectangle2f bounds;
        SharedTextureHandle texture;
        std::shared_ptr<GlMesh> mesh;

        Sprite(const Rectangle2f& bounds, SharedTextureHandle texture, std::shared_ptr<GlMesh> mesh);

        Matrix4f getTransform() const;
    };
}
