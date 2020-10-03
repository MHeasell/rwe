#pragma once

#include <rwe/geometry/CollisionMesh.h>
#include <rwe/render/GlMesh.h>
#include <rwe/render/VaoHandle.h>
#include <rwe/render/VboHandle.h>

namespace rwe
{
    struct SelectionMesh
    {
        CollisionMesh collisionMesh;
        GlMesh visualMesh;
    };
}
