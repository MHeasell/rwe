#ifndef RWE_SELECTIONMESH_H
#define RWE_SELECTIONMESH_H

#include "VaoHandle.h"
#include "VboHandle.h"
#include <rwe/geometry/CollisionMesh.h>

namespace rwe
{
    struct VisualSelectionMesh
    {
        VboHandle vbo;
        unsigned int vertexCount;
        VaoHandle vao;
    };

    struct SelectionMesh
    {
        CollisionMesh collisionMesh;
        VisualSelectionMesh visualMesh;
    };
}

#endif
