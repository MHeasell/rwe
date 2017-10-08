#include "UnitMesh.h"

namespace rwe
{

    void UnitMesh::render(GraphicsContext& context) const
    {
        context.pushMatrix();
        context.multiplyMatrix(Matrix4f::translation(origin));
        context.drawMesh(*mesh);
        context.popMatrix();
    }
}
