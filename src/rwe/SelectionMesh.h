#ifndef RWE_SELECTIONMESH_H
#define RWE_SELECTIONMESH_H

#include <rwe/geometry/CollisionMesh.h>
#include "VboHandle.h"
#include "VaoHandle.h"

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
