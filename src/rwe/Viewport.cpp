#include "Viewport.h"
#include <rwe/math/rwe_math.h>

namespace rwe
{
    Viewport::Viewport(int x, int y, unsigned int width, unsigned int height) : _x(x), _y(y), _width(width), _height(height)
    {
    }

    int Viewport::x() const
    {
        return _x;
    }

    int Viewport::y() const
    {
        return _y;
    }

    unsigned int Viewport::width() const
    {
        return _width;
    }

    unsigned int Viewport::height() const
    {
        return _height;
    }
}
