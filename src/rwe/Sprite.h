#ifndef RWE_SPRITE_H
#define RWE_SPRITE_H

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
        GlMesh mesh;

        Sprite(const Rectangle2f& bounds, SharedTextureHandle texture, GlMesh&& mesh);

        Matrix4f getTransform() const;
    };
}


#endif
