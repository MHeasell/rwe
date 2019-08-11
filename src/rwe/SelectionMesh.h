#pragma once

#include <rwe/GlMesh.h>
#include <rwe/VaoHandle.h>
#include <rwe/VboHandle.h>
#include <rwe/geometry/CollisionMesh.h>

namespace rwe
{
    struct SelectionMesh
    {
        CollisionMesh collisionMesh;
        GlMesh visualMesh;
    };
}
