#ifndef RWE_SPRITE_H
#define RWE_SPRITE_H

#include <rwe/geometry/Rectangle2f.h>
#include "GlTexturedMesh.h"

namespace rwe
{
    struct Sprite
    {
        Rectangle2f bounds;
        GlTexturedMesh mesh;

        Sprite(const Rectangle2f& bounds, GlTexturedMesh&& mesh);
    };
}


#endif
