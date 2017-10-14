#include "UnitMesh.h"

namespace rwe
{

    void UnitMesh::render(GraphicsContext& context) const
    {
        context.pushMatrix();
        context.multiplyMatrix(Matrix4f::translation(origin));

        context.drawMesh(*mesh);

        for (const auto& c : children)
        {
            c.render(context);
        }

        context.popMatrix();
    }
}
