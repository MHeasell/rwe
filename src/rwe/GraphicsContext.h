#ifndef RWE_GRAPHICSCONTEXT_H
#define RWE_GRAPHICSCONTEXT_H

#include <rwe/math/Vector3f.h>

namespace rwe
{
    class GraphicsContext
    {
    public:
        void drawTriangle(const Vector3f& a, const Vector3f& b, const Vector3f& c);

        void setBackgroundColor(float r, float g, float b);

        void clear();
    };
}

#endif
