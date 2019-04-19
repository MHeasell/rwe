#ifndef RWE_SPRITE_H
#define RWE_SPRITE_H

#include <rwe/GlTexturedMesh.h>
#include <rwe/geometry/Rectangle2f.h>
#include <rwe/math/Matrix4f.h>

namespace rwe
{
    struct Sprite
    {
        Rectangle2f bounds;
        GlTexturedMesh mesh;

        Sprite(const Rectangle2f& bounds, GlTexturedMesh&& mesh);

        Matrix4f getTransform() const;
    };
}


#endif
