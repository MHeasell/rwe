#include "Unit.h"

namespace rwe
{
    void Unit::render(GraphicsContext& context) const
    {
        context.pushMatrix();
        context.multiplyMatrix(Matrix4f::translation(position));
        mesh.render(context);
        context.popMatrix();
    }
}
