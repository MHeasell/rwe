#ifndef RWE_SELECTIONMESH_H
#define RWE_SELECTIONMESH_H

#include "VaoHandle.h"
#include "VboHandle.h"
#include <rwe/GlMesh.h>
#include <rwe/geometry/CollisionMesh.h>

namespace rwe
{
    struct SelectionMesh
    {
        CollisionMesh collisionMesh;
        GlMesh visualMesh;
    };
}

#endif
